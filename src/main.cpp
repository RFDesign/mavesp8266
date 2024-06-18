/****************************************************************************
 *
 * Copyright (c) 2015, 2016 Gus Grubba. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file main.cpp
 * ESP8266 Wifi AP, MavLink UART/UDP Bridge
 *
 * original @author Gus Grubba <mavlink@grubba.com>
 * TCP support by davidbuzz@gmail.com
 * txmod/900x bootloader and flashing support by davidbuzz@gmail.com
 */


#include "rfd900x.h"
#include "mavesp8266.h"
#include "mavesp8266_parameters.h"
#include "mavesp8266_gcs.h"
#include "mavesp8266_vehicle.h"
#include "mavesp8266_httpd.h"
#include "mavesp8266_component.h"
#include "FS.h" // for SPIFFS acccess
#include "sport.h"
#include "txmod_debug.h"
#include <XModem.h> // for firmware updates
#include <ESP8266mDNS.h>
#include "led.h"

// txmod reset button 
#define RESETGPIO 12
#define MAV_HEARTBEAT_PERIOD_MS 1000
#define TCP_CLIENT_CHECK_PERIOD_MS 200

// platformio doesn't seem to have F(), but has FPSTR and PSTR
#define F(string_literal) (FPSTR(PSTR(string_literal)))

//---------------------------------------------------------------------------------
//-- HTTP Update Status
class MavESP8266UpdateImp : public MavESP8266Update {
public:
    MavESP8266UpdateImp ()
        : _isUpdating(false)
    {

    }
    void updateStarted  ()
    {
        _isUpdating = true;
    }
    void updateCompleted()
    {
        //-- TODO
    }
    void updateError    ()
    {
        //-- TODO
    }
    bool isUpdating     () { return _isUpdating; }
private:
    bool _isUpdating;
};



//-- Singletons
IPAddress               localIP;
MavESP8266Component     Component;
MavESP8266Parameters    Parameters; // esp params

MavESP8266GCS           GCS;
MavESP8266Vehicle       Vehicle;
MavESP8266Httpd         updateServer;
MavESP8266UpdateImp     updateStatus;
MavESP8266Log           Logger;
MavESP8266Log           SoftLogger;

//---------------------------------------------------------------------------------
//-- Accessors
class MavESP8266WorldImp : public MavESP8266World {
public:
    MavESP8266Parameters*   getParameters   () { return &Parameters;    }
    MavESP8266Component*    getComponent    () { return &Component;     }
    MavESP8266Vehicle*      getVehicle      () { return &Vehicle;       }
    MavESP8266GCS*          getGCS          () { return &GCS;           }
    MavESP8266Log*          getLogger       () { return &Logger;        }
    MavESP8266Log*          getSoftLogger       () { return &SoftLogger;        }
};

MavESP8266WorldImp      World;

MavESP8266World* getWorld()
{
    return &World;
}

uint8 client_count = 0;

//---------------------------------------------------------------------------------
//-- Wait for a DHCPD client
void wait_for_client() {
    set_led_state(true);
    DEBUG_LOG("Waiting for a client...\n");
#ifdef ENABLE_DEBUG
    int wcount = 0;
#endif
    uint8 client_count = wifi_softap_get_station_num();
    while (!client_count) {
#ifdef ENABLE_DEBUG
        Serial1.print(".");
        if(++wcount > 80) {
            wcount = 0;
            Serial1.println();
        }
#endif
        delay(1000);
        toggle_led_state();
        client_count = wifi_softap_get_station_num();
    }
    DEBUG_LOG("Got %d client(s)\n", client_count);
    set_led_state(false);
}

int r900x_savesingle_param_and_verify_more(String prefix, String ParamID, String ParamVAL, bool save_and_reboot);
bool factory_reset_req = false;

//---------------------------------------------------------------------------------
//-- Reset all parameters whenever the reset gpio pin is active
void reset_interrupt(){
    factory_reset_req = true;
}

// count the number of user presses, and when it exceeds 5, reset to defaults.
volatile byte interruptCounter = 0;
volatile long first_press = 0;
void IRAM_ATTR count_interrupts() { 
    interruptCounter++;
    if ( first_press == 0 ) { first_press = millis(); } 
    if ( millis() - first_press > 5000) { first_press = 0; interruptCounter = 0; } 
    if ( interruptCounter >= 5 ) { reset_interrupt(); }// reboot after 5 presses
} 

#define PROTOCOL_TCP

#ifdef PROTOCOL_TCP
#include <WiFiClient.h>
WiFiServer tcpserver(23);
WiFiClient tcpclient;

#define bufferSize 512  
#define packTimeout 1
#define max_tcp_size 500 // never SEND any tcp packet over this size
#define max_serial_size 128 // never SEND any serial chunk over this size

// raw serial<->tcp passthrough buffers, if used.
uint8_t buf[bufferSize];

// variables for tcp-serial passthrough stats
long unsigned int msecs_counter = 0;
long int secs = 0;
long int stats_serial_in = 0;
long int stats_tcp_in = 0;
long int stats_serial_pkts = 0;
long int stats_tcp_pkts = 0;
long int largest_serial_packet = 0;
long int largest_tcp_packet = 0;
#endif
bool tcp_passthrumode = false;
bool isMavlinkEnabled = true;

//#define DEBUG_LOG debug_serial_println

void mav_bridges_setup() {

    //Parameters.setLocalIPAddress(localIP);
    IPAddress gcs_ip(localIP);
    //-- I'm getting bogus IP from the DHCP server. Broadcasting for now.
    gcs_ip[3] = 255;
    debug_serial_println(F("setting UDP client IP!"));

    // twiddle LEDs here so user sees they are connected
    for (int l = 0; l < 10; l++)
    {
        toggle_led_state();
        delay(100);
    }

    GCS.begin(&Vehicle, gcs_ip);
    debug_serial_println(F("GCS.begin finished"));
    Vehicle.begin(&GCS);
    debug_serial_println(F("Vehicle.begin finished"));
}

//---------------------------------------------------------------------------------
//-- Set things up
void setup() {
    r900x_initiate_serials();
    Parameters.begin();

#ifdef ENABLE_DEBUG
    //   We only use it for non debug because GPIO02 is used as a serial
    //   pin (TX) when debugging.
    Serial1.begin(57600);
    debug_serial_println(F("Serial1 output for DEBUG"));
#else
    setup_led(); 
    set_led_state(true);
    //-- Initialized RESETGPIO (Used for "Reset To Factory") 
    pinMode(RESETGPIO, INPUT_PULLUP); 
    attachInterrupt(RESETGPIO, count_interrupts, FALLING); 
#endif

    DEBUG_LOG("\nConfiguring access point...\n");
    DEBUG_LOG("Free Sketch Space: %u\n", ESP.getFreeSketchSpace());

    WiFi.disconnect(true);

    // get MAC address of adaptor as used in STA mode
    byte mac[6]; 
    WiFi.macAddress(mac);
    // as a string as well as its easier.
    String mac_half_s = half_mac2String(mac);

    // get MAC address of adaptor as used in AP mode
    byte mac_ap[6]; 
    WiFi.softAPmacAddress(mac_ap);

    //-- MDNS
    char mdnsName[256];
    sprintf(mdnsName, "TXMOD-%s",mac_half_s.c_str());

    if(Parameters.getWifiMode() == WIFI_MODE_STA){
        DEBUG_LOG("\nEntering station mode...\n");
        //-- Connect to an existing network
        WiFi.mode(WIFI_STA);
        WiFi.config(Parameters.getWifiStaIP(), Parameters.getWifiStaGateway(), Parameters.getWifiStaSubnet(), 0U, 0U);
        char * pwd = Parameters.getWifiStaPassword();
        pwd = strlen(pwd) < 8 ? NULL : pwd;
        WiFi.begin(Parameters.getWifiStaSsid(), pwd);

        //-- Wait a minute to connect
        for(int i = 0; i < 120 && WiFi.status() != WL_CONNECTED; i++) {
            #ifdef ENABLE_DEBUG
            //Serial.print(".");
            #endif
            delay(500);
            toggle_led_state();
        }
        if(WiFi.status() == WL_CONNECTED) {
            localIP = WiFi.localIP();
            set_led_state(false);
            WiFi.setAutoReconnect(true);
        } else {
            //-- Fall back to AP mode if no connection could be established
            set_led_state(true);
            WiFi.disconnect(true);
            Parameters.setWifiMode(WIFI_MODE_AP);
        }
    }

    if(Parameters.getWifiMode() == WIFI_MODE_AP){

        // look at the out-of-box SSID which is compiled-in as 'TXMOD' and revise it to TXMOD-xx-xx-xx-xx-xx-xx if needed.
        char *_tmp_wifi_ssid;
        _tmp_wifi_ssid = Parameters.getWifiSsid();
        int compare= strcmp(_tmp_wifi_ssid, "TXMOD");
        if ( compare == 0 ) {  // equal zero means it's still set as compiled-in 'TXMOD'
          Parameters.setWifiSsid(mdnsName);
        }

        DEBUG_LOG("\nEntering AP mode...\n");
        //-- Start AP
        WiFi.mode(WIFI_AP);
        WiFi.encryptionType(AUTH_WPA2_PSK);
        WiFi.softAP(Parameters.getWifiSsid(), Parameters.getWifiPassword(), Parameters.getWifiChannel());
        localIP = WiFi.softAPIP();
        //wait_for_client();
    }

    //-- Boost power to Max
    WiFi.setOutputPower(20.5);

    int retries = 5;
    while ( retries > 0) { 
       bool success = MDNS.begin(mdnsName);
       if ( success ) { retries = 0; } 
       else { 
        debug_serial_println("Error setting up MDNS responder!");
        delay(1000);
        retries --;
       }
    }
    MDNS.addService("_http", "_tcp", 80);    
    //MDNS.addService("tcp", "tcp", 23);

    #ifdef PROTOCOL_TCP
    debug_serial_println(F("Starting TCP Server on port 23"));
    tcpserver.begin(); // start TCP server 
    #endif

    //-- Initialize Comm Links
    DEBUG_LOG("Start WiFi Bridge\n");
    DEBUG_LOG("Local IP: %s\n", localIP.toString().c_str());

    Parameters.setLocalIPAddress(localIP);

    //-- Initialize Update Server
    updateServer.begin(&updateStatus); 

    //try at current/stock baud rate, 57600, first.
    debug_init();
    r900x_setup(true); // probe for 900x and if a new firware update is needed , do it.  CAUTION may hang in retries if 900x modem is NOT attached
    sport_setup();
    mav_bridges_setup();

    //see if the Mavlink flag is set within the RFD900X. If we fail
    //to read the cached value, the mavlink flag is presumed to be set
    int mavParam = r900x_readcachedparam(RFD_LOC_PAR, F("MAVLINK"));
    if (!mavParam) isMavlinkEnabled = false;

}

void client_check() { 
    uint8 x = wifi_softap_get_station_num();
    if ( client_count != x ) { 
        client_count = x;
        DEBUG_LOG("Got %d client(s)\n", client_count);  
    } 
} 

bool tcp_check() { 
#ifdef PROTOCOL_TCP

    if (!tcpclient.connected()) { 
        tcpclient = tcpserver.available();
        return (bool)tcpclient;
    } else { 
        return true;
    }
#endif
    return false;  // if compiled without tcp, always return false.
} 

void handle_tcp_and_serial_passthrough() {

    #ifdef PROTOCOL_TCP

        if ( millis() > msecs_counter +1000 ) { 
            msecs_counter = millis(); 
            secs++;

            stats_serial_in=0;
            stats_tcp_in=0;
            stats_serial_pkts=0;
            stats_tcp_pkts=0;
        }

        if(int avail = tcpclient.available()) {
            int bytes_read = 0;
            for (int i = 0; (i < avail) && (i < max_tcp_size); i++) {
                buf[i] = (char)tcpclient.read(); // read char from UART
                stats_tcp_in++;
                bytes_read++;
            }

            Serial.write(buf, bytes_read); 
            stats_serial_pkts++; 
            if ( avail > largest_tcp_packet ) largest_tcp_packet = avail;
        }

        if(int avail = Serial.available()) {
            int bytes_read = 0;
            for (int i = 0; (i < avail) && (i < max_tcp_size); i++) {
                buf[i] = (char)Serial.read(); // read char from UART
                stats_serial_in++;
                bytes_read++;
            }

            Vehicle.parseMessage(avail, buf);

            tcpclient.write((char*)buf, bytes_read);
            stats_tcp_pkts++;
            if (avail > largest_serial_packet) largest_serial_packet = avail;
        }
    #endif
}

void force_heartbeats(){
    static uint64_t next = 0;
    uint64_t now = millis();

    if (next <= now) {
        if (!GCS.heardFrom()) 
        {
            mavlink_message_t heartbeat;
            uint8_t system_id = 0xFF; // broadcast
            uint8_t component_id = MAV_COMP_ID_UART_BRIDGE;
            uint8_t type = MAV_TYPE_GCS;
            uint8_t autopilot = MAV_AUTOPILOT_GENERIC;
            uint8_t basemode = 0;
            uint8_t custommode = 0;
            uint8_t systemstatus = MAV_STATE_ACTIVE;

            mavlink_msg_heartbeat_pack(system_id, component_id, &heartbeat, type, autopilot, 
                basemode, custommode, systemstatus);

            Vehicle.sendMessage(&heartbeat);

            next = now + MAV_HEARTBEAT_PERIOD_MS;
        }
    }
}

void force_vehicle_datastream(){
    static bool sent_once = false;
    static uint32_t last_sent = 0;
    
    if (sent_once) {
        if (!Vehicle.heardFrom())
        {
            // allows messages to be sent once the
            // vehicle is heard from again
            sent_once = false;
        }
        return;
    }
    
    if (millis() - last_sent > 1000)
    {
        // PARAM_SET messages initiate data flow
        mavlink_message_t param_set;
        uint8_t system_id = 0xFF; // broadcast
        uint8_t component_id = MAV_COMP_ID_UART_BRIDGE;
        uint8_t target_system = 0; // broadcast
        uint8_t target_component = MAV_COMP_ID_AUTOPILOT1;

        const uint8_t mavStreams[] = {
            MAV_DATA_STREAM_RAW_SENSORS,
            MAV_DATA_STREAM_EXTENDED_STATUS,
            MAV_DATA_STREAM_RC_CHANNELS,
            MAV_DATA_STREAM_POSITION,
            MAV_DATA_STREAM_EXTRA1,
            MAV_DATA_STREAM_EXTRA2,
            MAV_DATA_STREAM_EXTRA3};

        const uint16_t mavRates[] = {0x04, 0x0a, 0x04, 0x0a, 0x04, 0x04, 0x04};
        // req_message_rate The requested interval between two messages of this type

        for (uint8_t i = 0; i < sizeof(mavStreams)/sizeof(mavStreams[0]); i++)
        {
            mavlink_msg_request_data_stream_pack(system_id, component_id, &param_set,
                target_system, target_component, mavStreams[i], mavRates[i], 1); // start_stop 1 to start sending, 0 to stop sending
                
            Vehicle.sendMessage(&param_set);
        }

        last_sent = millis();
        sent_once = true;
    }
}

//---------------------------------------------------------------------------------
//-- Main Loop
void loop() {
    static long time_next = 0;

    if(millis() > time_next){
        time_next = millis() + TCP_CLIENT_CHECK_PERIOD_MS;
        client_check();
    }

    if (tcp_check() ) {  // if a client connects to the tcp server, stop doing everything else and handle that
        if ( tcp_passthrumode == false ) {
            tcp_passthrumode = true;
            debug_serial_println(F("entered tcp-serial passthrough mode")); 
        }
        handle_tcp_and_serial_passthrough();

    } else { // do udp & mavlink comms by default and when no tcp clients are available

        if ( tcp_passthrumode == true ) { 
            tcp_passthrumode = false; 
            debug_serial_println(F("exited tcp-serial passthrough mode")); 
        }

        if(!updateStatus.isUpdating()) {
            if (Component.inRawMode()) {
                GCS.readMessageRaw();
                delay(0);
                Vehicle.readMessageRaw();
            } else {
                GCS.readMessage();
                delay(0);
                Vehicle.readMessage();
                toggle_led_state();
            }
        }
    }

    delay(0);

    if(isMavlinkEnabled && !updateStatus.isUpdating()) {
        force_vehicle_datastream();
        force_heartbeats();
    }

    updateServer.checkUpdates(); // aka webserver.handleClient()

    MDNS.update();

    delay(0);

    if (factory_reset_req) {

        debug_serial_println(F("attempting factory reset"));

        r900x_attempt_factory_reset();

        SPIFFS.remove(RFD_ENC_KEY);
        SPIFFS.remove(RFD_REM_PAR);
        SPIFFS.remove(RFD_LOC_PAR);
        SPIFFS.remove(RFD_REM_VER);
        SPIFFS.remove(RFD_LOC_VER);
        
        set_led_state(true);
        Parameters.resetToDefaults();
        Parameters.saveAllToEeprom();
        
        debug_serial_println(F("FACTORY RESET BUTTON PRESSED - wifi params defaulted!\n"));

        ESP.reset();
    }

    if (getWorld()->getParameters()->getSPORTenable()) sport_loop();
}

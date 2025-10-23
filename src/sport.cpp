#include <Arduino.h>
#include "mavesp8266.h"
#include "mavesp8266_parameters.h"
#include "rfd900x.h"
/*
=====================================================================================================================
     MavToPass  (Mavlink To FrSky Passthrough) Protocol Translator

 
     License and Disclaimer

 
  This software is provided under the GNU v2.0 License. All relevant restrictions apply. In case there is a conflict,
  the GNU v2.0 License is overriding. This software is provided as-is in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details. In no event will the authors and/or contributors be held liable for any 
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose, including commercial 
  applications, and to alter it and redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software.
  2. If you use this software in a product, an acknowledgment in the product documentation would be appreciated.
  3. Altered versions must be plainly marked as such, and must not be misrepresented as being the original software.
  4. This notice may not be removed or altered from any distribution.  

  By downloading this software you are agreeing to the terms specified in this page and the spirit of thereof.
    
    =====================================================================================================================

    Author: Eric Stockenstrom
    
        
    Inspired by original S.Port firmware by Rolf Blomgren
    
    Acknowledgements and thanks to Craft and Theory (http://www.craftandtheoryllc.com/) for
    the Mavlink / Frsky Passthru protocol

    Many thanks to yaapu for advice and testing, and his excellent LUA script

    Thanks also to athertop for advice and testing, and to florent for advice on working with FlightDeck

    Acknowledgement and thanks to the author of, and contributors to, mavESP8266 serial/Wifi bridge
    
    ======================================================================================================

    See: https://github.com/zs6buj/MavlinkToPassthru/wiki
    
    While the DragonLink and Orange UHF Long Range RC and telemetry radio systems deliver 
    a two-way Mavlink link, the FrSky Taranis and Horus hand-held RC controllers expect
    to receive FrSky S.Port protocol telemetry for display on their screen.  Excellent 
    firmware is available to convert Mavlink to the native S.Port protocol, however the 
    author is unaware of a suitable solution to convert to the Passthru protocol. 

    This protocol translator has been especially tailored to work with the excellent LUA 
    display interface from yaapu for the FrSky Horus, Taranis and QX7 controllers. 
    https://github.com/yaapu/FrskyTelemetryScript . The translator also works with the 
    popular FlightDeck product.
    
    The firmware translates APM or PX4 Mavlink telemetry to FrSky S.Port passthru telemetry, 
    and is designed to run on an ESP32, ESP8266 or Teensy 3.2. The ESP32 implementation supports
    WiFI, Bluetooth and SD card I/O, while the ESP8266 supports only WiFi and SD I/O into and 
    out of the translator. So for example, Mavlink telemetry can be fed directly into Mission
    Planner, QGround Control or an antenna tracker.

    The performance of the translator on the ESP32 platform is superior to that of the other boards.
    However, the Teensy 3.x is much smaller and fits neatly into the back bay of a Taranis or Horus
    transmitter. The STM32F103C boards are no longer supported.

    The PLUS version adds additional sensor IDs to Mavlink Passthru protocol DIY range

    The translator can work in one of three modes: Ground_Mode, Air_Mode or Relay_Mode

    Ground_Mode
    In ground mode, it is located in the back of the Taranis/Horus. Since there is no FrSky receiver
    to provide sensor polling, a routine in the firmware emulates FrSky receiver sensor polling. (It
    pretends to be a receiver for polling purposes). 
   
    Un-comment this line       #define Ground_Mode      like this.

    Air_Mode
    In air mode, it is located on the aircraft between the FC and a Frsky receiver. It translates 
    Mavlink out of a Pixhawk and feeds passthru telemetry to the Frsky receiver, which sends it 
    to the Taranis on the ground. In this situation it responds to the FrSky receiver's sensor 
    polling. The APM firmware can deliver passthru telemetry directly without this translator, but as 
    of July 2019 the PX4 Pro firmware cannot, and therefor requires this translator. 
   
    Un-comment this line      #define Air_Mode    like this
   
    Relay_Mode
    Consider the situation where an air-side LRS UHF tranceiver (trx) (like the DragonLink or Orange), 
    communicates with a matching ground-side UHF trx located in a "relay" box using Mavlink 
    telemetry. The UHF trx in the relay box feeds Mavlink telemtry into our passthru translator, and 
    the ctranslator feeds FrSky passthru telemtry into the FrSky receiver (like an XSR), also 
    located in the relay box. The XSR receiver (actually a tranceiver - trx) then communicates on 
    the public 2.4GHz band with the Taranis on the ground. In this situation the translator need not 
    emulate sensor polling, as the FrSky receiver will provide it. However, the translator must 
    determine the true rssi of the air link and forward it, as the rssi forwarded by the FrSky 
    receiver in the relay box will incorrectly be that of the short terrestrial link from the relay
    box to the Taranis.  To enable Relay_Mode :
    Un-comment this line      #define Relay_Mode    like this

    From version 2.12 the target mpu is selected automatically

    Battery capacities in mAh can be 
   
    1 Requested from the flight controller via Mavlink
    2 Defined within this firmware  or 
    3 Defined within the LUA script on the Taranis/Horus. This is the prefered method.
     
    N.B!  The dreaded "Telemetry Lost" enunciation!

    The popular LUA telemetry scripts use RSSI to determine that a telemetry connection has been successfully established 
    between the 'craft and the Taranis/Horus. Be sure to set-up RSSI properly before testing the system.


    =====================================================================================================================


   Connections to ESP32 or ESP8266 boards depend on the board variant

    Go to config.h tab and look for "S E L E C T   E S P   B O A R D   V A R I A N T" 

    
   Connections to Teensy3.2 are:
    0) USB                         Flashing and serial monitor for debug
    1) SPort S     -->tx1 Pin 1    S.Port out to XSR  or Taranis bay, bottom pin
    2) Mavlink_In  <--rx2 Pin 9    Mavlink source to Teensy - FC_Mav_rxPin
    3) Mavlink_In  -->tx2 Pin 10   Mavlink source to Taranis
    4) Mavlink_Out <--rx3 Pin 7    Optional feature - see #defined
    5) Mavlink_Out -->tx3 Pin 8    Optional feature - see #defined
    6) MavStatusLed       Pin 13   BoardLed
    7) BufStatusLed  1
    8) Vcc 3.3V !
    9) GND

    NOTE : STM32 support is deprecated as of 2020-02-27 v2.56.2
*/
//    =====================================================================================================================

#ifndef ESP8266      // Tennsy 3.x, ESP32 .....
  //#undef F           // F defined in c_library_v2\mavlink_sha256.h AND teensy3/WString.h
#endif

#include "sport.h"
#include "mavesp8266.h"
#include <CircularBuffer.h>
#include <mavlink_types.h>
#include <common/mavlink.h>
#include <ardupilotmega/ardupilotmega.h>
#include "txmod_debug.h"
#include "sport/autopilotselection.h"
#include <SoftwareSerial.h>

SoftwareSerial frSerial;//(-1, Fr_txPin, true, 256);

uint32_t  sens_buf_full_count = 0;

#if (Battery_mAh_Source == 1)  // Get battery capacity from the FC
  bool      ap_bat_paramsReq = false;
  bool      parm_msg_shown = false;
#endif
bool      ap_bat_paramsRead=false; 
uint8_t   app_count=0;

bool      homGood = false;      

uint32_t  rs_millis=0;    // last time an RSSI figure has been received
uint32_t  sport_millis=0;  
#ifdef Data_Streams_Enabled 
uint32_t  rds_millis=0;
#endif
uint32_t  em_millis=0;
#if defined Send_Sensor_Health_Messages
uint32_t  health_millis = 0;
#endif





float   lon1,lat1,lon2,lat2,alt1,alt2;  
//=================================================================================================  
// 4D Location vectors
 struct Location {
  float lat; 
  float lon;
  float alt;
  float hdg;
  };
volatile struct Location hom     = {
  0,0,0,0};   // home location

volatile struct Location cur      = {
  0,0,0,0};   // current location  
   
struct Loc2D {
  float     lat; 
  float     lon;
  };
  
//=========================================== M A V L I N K =============================================    

uint16_t            len;

// Mavlink Messages

// Mavlink Header
uint8_t    ap_sysid;
uint8_t    ap_compid;
uint8_t    ap_targcomp;
uint8_t    ap_targsys;     //   System ID of target system - outgoing to FC
uint8_t    mvType;

// Message #0  HEARTHBEAT 
uint8_t    ap_type_tmp = 0;              // hold the type until we know HB not from GCS or Tracker
uint8_t    ap_type = 0;
uint8_t    ap_autopilot = 0;
uint8_t    ap_base_mode = 0;
uint32_t   ap_custom_mode = 0;
bool       px4_flight_stack = false;
uint8_t    px4_main_mode = 0;
uint8_t    px4_sub_mode = 0;

// Message #0  GCS HEARTHBEAT 

uint8_t    gcs_type = 0;
uint8_t    gcs_autopilot = 0;
uint8_t    gcs_base_mode = 0;
uint32_t   gcs_custom_mode = 0;
uint8_t    gcs_system_status = 0;
uint8_t    gcs_mavlink_version = 0;

// Message #0  Outgoing HEARTHBEAT 
uint8_t    apo_sysid;
uint8_t    apo_compid;
uint8_t    apo_targcomp;
uint8_t    apo_mission_type;              // Mav2
uint8_t    apo_type = 0;
uint8_t    apo_autopilot = 0;
uint8_t    apo_base_mode = 0;
uint32_t   apo_custom_mode = 0;
uint8_t    apo_system_status = 0;

// Message # 1  SYS_status 
uint32_t   ap_onboard_control_sensors_health;  //Bitmap  0: error. Value of 0: error. Value of 1: healthy.
uint16_t   ap_voltage_battery1 = 0;    // 1000 = 1V
int16_t    ap_current_battery1 = 0;    //  10 = 1A
uint8_t    ap_ccell_count1= 0;

// Message # 2  SYS_status 
uint64_t  ap_time_unix_usec;          // us  Timestamp (UNIX epoch time).
uint32_t  ap_time_boot_ms;            // ms  Timestamp (time since system boot)

// Message #20 PARAM_REQUEST_READ    // outgoing request to read the onboard parameter with the param_id string id
uint8_t  gcs_targsys;            //   System ID
char     gcs_req_param_id[16];   //  Onboard parameter id, terminated by NULL if the length is less than 16 human-readable chars and WITHOUT null termination (NULL) byte if the length is exactly 16 chars - applications have to provide 16+1 bytes storage if the ID is stored as string
int16_t  gcs_req_param_index;    //  Parameter index. Send -1 to use the param ID field as identifier (else the param id will be ignored)
// param_index . Send -1 to use the param ID field as identifier (else the param id will be ignored)

float ap_bat1_capacity;
float ap_bat2_capacity;

// Message #21 PARAM_REQUEST_LIST 
//  Generic Mavlink Header defined above
  
// Message #22 PARAM_VALUE
char     ap_param_id [16]; 
float    ap_param_value;
uint8_t  ap_param_type;  
uint16_t ap_param_count;              //  Total number of onboard parameters
int16_t ap_param_index;              //  Index of this onboard parameter

// Message #24  GPS_RAW_INT 
uint8_t    ap_fixtype = 3;            // 0= No GPS, 1=No Fix, 2=2D Fix, 3=3D Fix, 4=DGPS, 5=RTK_Float, 6=RTK_Fixed, 7=Static, 8=PPP
uint8_t    ap_sat_visible = 0;        // numbers of visible satelites
int32_t    ap_lat24 = 0;              // 7 assumed decimal places
int32_t    ap_lon24 = 0;              // 7 assumed decimal places
int32_t    ap_amsl24 = 0;             // 1000 = 1m
uint16_t   ap_eph;                    // GPS HDOP horizontal dilution of position (unitless)
uint16_t   ap_vel;                    // GPS ground speed (m/s * 100) cm/s
uint16_t   ap_cog;                    // Course over ground in degrees * 100, 0.0..359.99 degrees
// mav2

// Message #26  SCALED_IMU
// mav2
int16_t ap26_temp = 0;             // cdegC

// Message #27 RAW IMU 
int32_t   ap27_xacc = 0;
int32_t   ap27_yacc = 0;
int32_t   ap27_zacc = 0;
int16_t   ap27_xgyro = 0;
int16_t   ap27_ygyro = 0;
int16_t   ap27_zgyro = 0;
int16_t   ap27_xmag = 0;
int16_t   ap27_ymag = 0;
int16_t   ap27_zmag = 0;
// mav2
int8_t    ap27_id = 0;
int16_t   ap27_temp = 0;             // cdegC

// Message #29 SCALED_PRESSURE
float      ap_press_abs;         // Absolute pressure (hectopascal)
float      ap_press_diff;        // Differential pressure 1 (hectopascal)
int16_t    ap_temperature;       // Temperature measurement (0.01 degrees celsius)

// Message ATTITUDE ( #30 )
float ap_roll;                   // Roll angle (rad, -pi..+pi)
float ap_pitch;                  // Pitch angle (rad, -pi..+pi)
float ap_yaw;                    // Yaw angle (rad, -pi..+pi)

// Message GLOBAL_POSITION_INT ( #33 ) (Filtered)
int32_t ap_lat33;          // Latitude, expressed as degrees * 1E7
int32_t ap_lon33;          // Longitude, expressed as degrees * 1E7
int32_t ap_amsl33;         // Altitude above mean sea level (millimeters)
int32_t ap_alt_ag;         // Altitude above ground (millimeters)
uint16_t ap_gps_hdg;           // Vehicle heading (yaw angle) in degrees * 100, 0.0..359.99 degrees

// Message #35 RC_CHANNELS_RAW
uint8_t ap_rssi;

// Message #39 Mission_Item
//  Generic Mavlink Header defined above
uint16_t  ap_ms_seq;            // Sequence
uint8_t   ap_mission_type;      // MAV_MISSION_TYPE - Mavlink 2

// Message #40 Mission_Request
//  Generic Mavlink Header defined above
//uint8_t   ap_mission_type;  

// Message #42 Mission_Current
//  Generic Mavlink Header defined above
bool ap_ms_current_flag = false;

// Message #43 Mission_Request_List
//  Generic Mavlink Header defined above
bool ap_ms_list_req = false;

// Message #44 Mission_Count
//  Generic Mavlink Header defined above
uint8_t   ap_mission_count = 0;
bool      ap_ms_count_ft = true;

// Message #51 Mission_Request_Int    From GCS to FC - Request info on mission seq #

uint8_t    gcs_target_system;    // System ID
uint8_t    gcs_target_component; // Component ID
uint16_t   gcs_seq;              // Sequence #
uint8_t    gcs_mission_type;      

// Message #62 Nav_Controller_Output
int16_t   ap_target_bearing;     // Bearing to current waypoint/target
uint16_t  ap_wp_dist;            // Distance to active waypoint
float     ap_xtrack_error;       // Current crosstrack error on x-y plane

// Message #65 RC_Channels
bool      ap_rc_flag = false;    // true when rc record received
uint8_t   ap_chcnt; 
uint16_t  ap_chan_raw[18];       // 16 + 2 channels, [0] thru [17] 

// Message #73 Mission_Item_Int
#if defined Mav_Debug_All || defined Mav_Debug_Mission
uint8_t   ap73_target_system;    
uint8_t   ap73_target_component;    
uint16_t  ap73_seq;             // Waypoint ID (sequence number)
uint8_t   ap73_frame;           // MAV_FRAME The coordinate system of the waypoint.
uint16_t  ap73_command;         // MAV_CMD The scheduled action for the waypoint.
uint8_t   ap73_current;         // false:0, true:1
uint8_t   ap73_autocontinue;    // Autocontinue to next waypoint
float     ap73_param1;          // PARAM1, see MAV_CMD enum
float     ap73_param2;          // PARAM2, see MAV_CMD enum
float     ap73_param3;          // PARAM3, see MAV_CMD enum
float     ap73_param4;          // PARAM4, see MAV_CMD enum
int32_t   ap73_x;               // PARAM5 / local: x position in meters * 1e4, global: latitude in degrees * 10^7
int32_t   ap73_y;               // PARAM6 / y position: local: x position in meters * 1e4, global: longitude in degrees *10^7
float     ap73_z;               // PARAM7 / z position: global: altitude in meters (relative or absolute, depending on frame.
uint8_t   ap73_mission_type;    // Mav2   MAV_MISSION_TYPE  Mission type.
#endif

// Message #74 VFR_HUD  
float    ap_hud_air_spd;
float    ap_hud_grd_spd;
int16_t  ap_hud_hdg;
uint16_t ap_hud_throt;
float    ap_hud_bar_alt;   
float    ap_hud_climb;        

// Message  #125 POWER_status 
uint16_t  ap_Vcc;                 // 5V rail voltage in millivolts
uint16_t  ap_Vservo;              // servo rail voltage in millivolts
uint16_t  ap_flags;               // power supply status flags (see MAV_POWER_status enum)
/*
 * MAV_POWER_status
Power supply status flags (bitmask)
1   MAV_POWER_status_BRICK_VALID  main brick power supply valid
2   MAV_POWER_status_SERVO_VALID  main servo power supply valid for FMU
4   MAV_POWER_status_USB_CONNECTED  USB power is connected
8   MAV_POWER_status_PERIPH_OVERCURRENT peripheral supply is in over-current state
16  MAV_POWER_status_PERIPH_HIPOWER_OVERCURRENT hi-power peripheral supply is in over-current state
32  MAV_POWER_status_CHANGED  Power status has changed since boot
 */

// Message  #147 BATTERY_status 
uint8_t      ap_battery_id;       
uint8_t      ap_battery_function;
uint8_t      ap_bat_type;  
int16_t      ap_bat_temperature;    // centi-degrees celsius
uint16_t     ap_voltages[10];       // cell voltages in millivolts 
int16_t      ap_current_battery;    // in 10*milliamperes (1 = 10 milliampere)
int32_t      ap_current_consumed;   // mAh
int32_t      ap_energy_consumed;    // HectoJoules (intergrated U*I*dt) (1 = 100 Joule)
int8_t       ap_battery_remaining;  // (0%: 0, 100%: 100)
int32_t      ap_time_remaining;     // in seconds
uint8_t      ap_charge_state;     

// Message #166 RADIO see #109


// Message #173 RANGEFINDER 
float ap_range; // m

// Message #181 BATTERY2 
uint16_t   ap_voltage_battery2 = 0;    // 1000 = 1V
int16_t    ap_current_battery2 = 0;    //  10 = 1A
uint8_t    ap_cell_count2 = 0;

// Message #253 STATUSTEXT
 uint8_t   ap_severity;
 char      ap_text[60];  // 50 plus padding
 uint8_t   ap_txtlth;
 bool      ap_simple=0;
 
//=====================================  F  R  S  K  Y  ===========================================

// 0x800 GPS
uint8_t ms2bits;
uint32_t fr_lat = 0;
uint32_t fr_lon = 0;

// 0x5000 Text Msg
uint32_t fr_textmsg;
char     fr_text[60];
uint8_t  fr_severity;
uint8_t  fr_txtlth;
char     fr_chunk[4];
uint8_t  fr_chunk_num;
uint8_t  fr_chunk_pntr = 0;  // chunk pointer
char     fr_chunk_print[5];

// 0x5001 AP Status
uint8_t fr_flight_mode;
uint8_t fr_simple;

uint8_t fr_land_complete;
uint8_t fr_armed;
uint8_t fr_bat_fs;
uint8_t fr_ekf_fs;
uint8_t fr_imu_temp;

// 0x5002 GPS Status
uint8_t fr_numsats;
uint8_t fr_gps_status;           // part a
uint8_t fr_gps_adv_status;       // part b
uint8_t fr_hdop;
uint32_t fr_amsl;

uint8_t neg;

//0x5003 Batt
uint16_t fr_bat1_volts;
uint16_t fr_bat1_amps;
uint16_t fr_bat1_mAh;

// 0x5004 Home
uint16_t fr_home_dist;
int16_t  fr_home_angle;       // degrees
int16_t  fr_home_arrow;       // 0 = heading pointing to home, unit = 3 degrees
int16_t  fr_home_alt;

short fr_pwr;

// 0x5005 Velocity and yaw
uint32_t fr_velyaw;
float fr_vy;    // climb in decimeters/s
float fr_vx;    // groundspeed in decimeters/s
float fr_yaw;   // heading units of 0.2 degrees

// 0x5006 Attitude and range
uint16_t fr_roll;
uint16_t fr_pitch;
uint16_t fr_range;

// 0x5007 Parameters  
uint8_t  fr_param_id ;
uint32_t fr_param_val;
uint32_t fr_frame_type;
uint32_t fr_bat1_capacity;
uint32_t fr_bat2_capacity;
uint32_t fr_mission_count;
bool     fr_paramsSent = false;

//0x5008 Batt2
float fr_bat2_volts;
float fr_bat2_amps;
uint16_t fr_bat2_mAh;

//0x5009 Servo_raw         // 4 ch per frame
uint8_t  frPort; 
int8_t   fr_sv[5];       

//0x50F1 HUD
float    fr_air_spd;       // dm/s
uint16_t fr_throt;         // 0 to 100%
float    fr_bar_alt;       // metres

//0x50F2 Missions       
uint16_t  fr_ms_seq;                // WP number
uint16_t  fr_ms_dist;               // To next WP  
float     fr_ms_xtrack;             // Cross track error in metres
float     fr_ms_target_bearing;     // Direction of next WP
float     fr_ms_cog;                // Course-over-ground in degrees
int8_t    fr_ms_offset;             // Next WP bearing offset from COG

//0x50F3 Wind Estimate      
uint16_t  fr_wind_speed;            // dm/s
uint16_t  fr_direction;             // Wind direction relative to yaw, deg / 3

//=================================================================================================   
//                            Ring and Sensor Buffers 
//=================================================================================================   
// Give the ESP32 more space, because it has much more RAM
#ifdef ESP32
  CircularBuffer<mavlink_message_t, 30> MavRingBuff;
#else 
  CircularBuffer<mavlink_message_t, 10> MavRingBuff;
#endif

// Scheduler buffer
 typedef struct  {
  uint16_t   id;
  uint8_t    subid;
  uint32_t   millis; // mS since boot
  uint32_t   payload;
  bool       inuse;
  } sb_t;

  // Give the sensor table more space when status_text messages sent three times
  #if defined Send_status_Text_3_Times
     const uint16_t sb_rows = 300;  // possible unsent sensor ids at any moment 
  #else 
     const uint16_t sb_rows = 130;  
  #endif

  sb_t sr, sb[sb_rows];
  char safety_padding[10];
  uint16_t sb_unsent;  // how many rows in use

  enum SPortMode { rx , tx };
  SPortMode mode, modeNow;

  bool      pb_rx = true;  // For PrintByte() direction indication
  
//=================================================================================================   
//                     F O R W A R D    D E C L A R A T I O N S
//=================================================================================================

// Forward declarations
void SPort_Init(void);
void PackSensorTable(uint16_t, uint8_t);
void RB_To_Decode_To_SPort_and_GCS();
void MavToRingBuffer(mavlink_message_t*);
void DecodeOneMavFrame(mavlink_message_t);
void SPort_Blind_Inject_Packet();
void MarkHome();
uint32_t Get_Volt_Average1(uint16_t);
uint32_t Get_Volt_Average2(uint16_t);
uint32_t Get_Current_Average1(uint16_t);
uint32_t Get_Current_Average2(uint16_t); 
void Accum_mAh1(uint32_t);
void Accum_mAh2(uint32_t);
void Accum_Volts1(uint32_t); 
void Accum_Volts2(uint32_t); 
void SPort_Inject_Packet();
void SPort_SendByte(uint8_t, bool);
void SPort_SendDataFrame(uint8_t, uint16_t, uint32_t);
void PackLat800(uint16_t);
void PackLon800(uint16_t);
void PackMultipleTextChunks_5000(uint16_t);
void Pack_AP_status_5001(uint16_t);
void Pack_GPS_status_5002(uint16_t);
void Pack_Bat1_5003(uint16_t);
void Pack_Home_5004(uint16_t);
void Pack_VelYaw_5005(uint16_t);
void Pack_Atti_5006(uint16_t);
void Pack_Parameters_5007(uint16_t);
void Pack_Bat2_5008(uint16_t);
void Pack_WayPoint_5009(uint16_t);
void Pack_Servo_Raw_50F1(uint16_t);
void Pack_VFR_Hud_50F2(uint16_t );
void Pack_Rssi_F101(uint16_t );
void CheckByteStuffAndSend(uint8_t);
uint32_t createMask(uint8_t, uint8_t);
uint32_t Abs(int32_t);
float RadToDeg (float );
uint16_t prep_number(int32_t, uint8_t, uint8_t);
int16_t Add360(int16_t, int16_t);
float wrap_360(int16_t);
int8_t PWM_To_63(uint16_t);
void DisplayByte(byte);
uint8_t PX4FlightModeNum(uint8_t, uint8_t);

//=================================================================================================
//=================================================================================================   
//                                      S   E   T   U  P 
//=================================================================================================
//=================================================================================================
void sport_setup()  {

  debug_serial_print("Target Board is ");
   
  #if (defined ESP8266) 
    debug_serial_print("ESP8266 / Variant is ");
    #if (ESP8266_Variant == 2)
      debug_serial_println("ESP-F - RFD900X TX-MOD");
    #endif       
  #endif

  debug_serial_println("Ground Mode");

  #if (Battery_mAh_Source == 1)  
    debug_serial_println("Battery_mAh_Source = 1 - Get battery capacities from the FC");
  #elif (Battery_mAh_Source == 2)
    debug_serial_println("Battery_mAh_Source = 2 - Define battery capacities in this firmware");  
  #elif (Battery_mAh_Source == 3)
    debug_serial_println("Battery_mAh_Source = 3 - Define battery capacities in the LUA script");
  #else
    #error You must define at least one Battery_mAh_Source. Please correct.
  #endif            

  #if (SPort_Serial == 1) 
    debug_serial_println("Using Serial_1 for S.Port");     
  #else
    debug_serial_println("Using Serial_3 for S.Port");
  #endif  
  
  #ifndef RSSI_Override
    debug_serial_println("RSSI Automatic Select");
  #else
    debug_serial_println("RSSI Override for testing = 70%");
  #endif

//=================================================================================================   
//                                    S E T U P   S E R I A L
//=================================================================================================  

  SPort_Init();
 
//=================================================================================================   
//                                    S E T U P   O T H E R
//=================================================================================================   
  homGood = false;     
  sport_millis = millis();
  #ifdef Data_Streams_Enabled 
  rds_millis=millis();
  #endif
  em_millis=millis();
  #if defined Send_Sensor_Health_Messages
  health_millis = millis();
  #endif
   
}
//================================================================================================= 
//================================================================================================= 
//                                   M  A  I  N    L  O  O  P
//================================================================================================= 
//================================================================================================= 

void sport_loop() {
  static uint32_t  param_millis = 0;
  static bool ap_rssi_ft = true; // first rssi connection

  if (ap_rssi > 0) {
    if (ap_rssi_ft) {  // first time we have an rc connection
      ap_rssi_ft = false;
      PackSensorTable(0x5007, 0);  // Send basic ap parameters (like frame type) 
      delay (10);
      PackSensorTable(0x5007, 0);  // 
      delay (10);
      PackSensorTable(0x5007, 0);  // three times
    }
    else if ((millis () - param_millis) > 5000) {
      param_millis = millis();
      PackSensorTable(0x5007, 0);
    }
  }

  static uint32_t rssi_millis = 0;
  bool rssiGood = (millis() - rs_millis) < 2000; // rssi should only be reported if has updated in the last 2 seconds

  if (rssiGood && (millis() - rssi_millis > 200)) {
    PackSensorTable(0xF101, 0);   // 0xF101 RSSI 
    rssi_millis = millis(); 
  }   

  if (millis() - sport_millis > 1) {   // main timing loop for S.Port
    RB_To_Decode_To_SPort_and_GCS();
  }

  #ifdef Data_Streams_Enabled 
    if(millis()-rds_millis > 5000) {
    rds_millis=millis();
    debug_serial_println("Requesting data streams"); 
    RequestDataStreams();   // must have Teensy Tx connected to Taranis/FC rx  (When SRx not enumerated)
    }
  #endif 

  #if defined Request_Missions_From_FC || defined Request_Mission_Count_From_FC
    if (!ap_ms_list_req) {
      RequestMissionList();  //  #43
      ap_ms_list_req = true;
    }
  #endif

  #if (Battery_mAh_Source == 1)  // Get battery capacity from the FC
  // Request battery capacity params 
    if (!ap_bat_paramsReq) {
      Param_Request_Read(356);    // Request Bat1 capacity   do this twice in case of lost frame
      Param_Request_Read(356);    
      Param_Request_Read(364);    // Request Bat2 capacity
      Param_Request_Read(364);    
      debug_serial_println("Battery capacities requested");
      ap_bat_paramsReq = true;
    } else {
      if (ap_bat_paramsRead &&  (!parm_msg_shown)) {
        parm_msg_shown = true; 
        debug_serial_println("Battery params successfully read"); 
      }
    } 
  #endif 
 
}
//================================================================================================= 
//                               E N D   O F   M  A  I  N    L  O  O  P
//================================================================================================= 

/**
 * Handle a mavlink message from the UAS
 */
void sport_handle_mavlink(mavlink_message_t * msg) {
    //debug_serial_println("mav msg rcvd: "+String(msg->msgid));
    sport::autopilotselection::Ingest(msg);
    if (sport::autopilotselection::IsThisFromTheChosenAutoPilot(msg) || 
      msg->msgid == MAVLINK_MSG_ID_RADIO_STATUS)
    {
      MavToRingBuffer(msg);
      sport::autopilotselection::IncrementCount();
    }
    else
    {
      //debug_println("Message not from AP " + String(msg->sysid) + " " + String(msg->compid));
    }
}
//=================================================================================================  
void PrintByte(byte b) {
  if (b == 0x7E) {
    debug_serial_println();
    //clm = 0;
  }
  if (b<=0xf) debug_serial_print("0");
  debug_serial_print(String(b,HEX));
  if (pb_rx) {
    debug_serial_print("<");
  } else {
    debug_serial_print(">");
  }
  /*
  clm++;
  if (clm > 30) {
    debug_serial_println();
    clm=0;
  }
  */
}
//=================================================================================================  
void PrintMavBuffer(const void *object){

    const unsigned char * const bytes = static_cast<const unsigned char *>(object);
  int j;

uint8_t   tl;

uint8_t mavNum;

//Mavlink 1 and 2
uint8_t mav_magic;               // protocol magic marker
uint8_t mav_len;                 // Length of payload

//uint8_t mav_incompat_flags;    // MAV2 flags that must be understood
//uint8_t mav_compat_flags;      // MAV2 flags that can be ignored if not understood

//uint8_t mav_seq;                // Sequence of packet
//uint8_t mav_sysid;            // ID of message sender system/aircraft
//uint8_t mav_compid;           // ID of the message sender component
uint8_t mav_msgid;            
/*
uint8_t mav_msgid_b1;           ///< first 8 bits of the ID of the message 0:7; 
uint8_t mav_msgid_b2;           ///< middle 8 bits of the ID of the message 8:15;  
uint8_t mav_msgid_b3;           ///< last 8 bits of the ID of the message 16:23;
uint8_t mav_payload[280];      ///< A maximum of 255 payload bytes
uint16_t mav_checksum;          ///< X.25 CRC
*/

  
  if ((bytes[0] == 0xFE) || (bytes[0] == 0xFD)) {
    j = -2;   // relative position moved forward 2 places
  } else {
    j = 0;
  }
   
  mav_magic = bytes[j+2];
  if (mav_magic == 0xFE) {  // Magic / start signal
    mavNum = 1;
  } else {
    mavNum = 2;
  }
/* Mav1:   8 bytes header + payload
 * magic
 * length
 * sequence
 * sysid
 * compid
 * msgid
 */
  
  if (mavNum == 1) {
    debug_serial_print("mav1: /");

    if (j == 0) {
      PrintByte(bytes[0]);   // CRC1
      PrintByte(bytes[1]);   // CRC2
      debug_serial_print("/");
      }
    mav_magic = bytes[j+2];   
    mav_len = bytes[j+3];
 //   mav_incompat_flags = bytes[j+4];;
 //   mav_compat_flags = bytes[j+5];;
//    mav_seq = bytes[j+6];
 //   mav_sysid = bytes[j+7];
 //   mav_compid = bytes[j+8];
    mav_msgid = bytes[j+9];

    //debug_serial_print(TimeString(millis()/1000)); debug_serial_print(": ");
  
    //debug_serial_print("seq="); debug_serial_print(mav_seq); debug_serial_print("\t"); 
    debug_serial_print("len="); debug_serial_print(mav_len); debug_serial_print("\t"); 
    debug_serial_print("/");
    for (int i = (j+2); i < (j+10); i++) {  // Print the header
      PrintByte(bytes[i]); 
    }
    
    debug_serial_print("  ");
    debug_serial_print("#");
    debug_serial_print(mav_msgid);
    if (mav_msgid < 100) debug_serial_print(" ");
    if (mav_msgid < 10)  debug_serial_print(" ");
    debug_serial_print("\t");
    
    tl = (mav_len+10);                // Total length: 8 bytes header + Payload + 2 bytes CRC
 //   for (int i = (j+10); i < (j+tl); i++) {  
    for (int i = (j+10); i <= (tl); i++) {    
     PrintByte(bytes[i]);     
    }
    if (j == -2) {
      debug_serial_print("//");
      PrintByte(bytes[mav_len + 8]); 
      PrintByte(bytes[mav_len + 9]); 
      }
    debug_serial_println("//");  
  } else {

/* Mav2:   10 bytes
 * magic
 * length
 * incompat_flags
 * mav_compat_flags 
 * sequence
 * sysid
 * compid
 * msgid[11] << 16) | [10] << 8) | [9]
 */
    
    debug_serial_print("mav2:  /");
    if (j == 0) {
      PrintByte(bytes[0]);   // CRC1
      PrintByte(bytes[1]);   // CRC2 
      debug_serial_print("/");
    }
    mav_magic = bytes[2]; 
    mav_len = bytes[3];
//    mav_incompat_flags = bytes[4]; 
//    mav_compat_flags = bytes[5];
//    mav_seq = bytes[6];
//    mav_sysid = bytes[7];
//    mav_compid = bytes[8]; 
    mav_msgid = (bytes[11] << 16) | (bytes[10] << 8) | bytes[9]; 

    //debug_serial_print(TimeString(millis()/1000)); debug_serial_print(": ");

    //debug_serial_print("seq="); debug_serial_print(mav_seq); debug_serial_print("\t"); 
    debug_serial_print("len="); debug_serial_print(mav_len); debug_serial_print("\t"); 
    debug_serial_print("/");
    for (int i = (j+2); i < (j+12); i++) {  // Print the header
     PrintByte(bytes[i]); 
    }

    debug_serial_print("  ");
    debug_serial_print("#");
    debug_serial_print(mav_msgid);
    if (mav_msgid < 100) debug_serial_print(" ");
    if (mav_msgid < 10)  debug_serial_print(" ");
    debug_serial_print("\t");

 //   tl = (mav_len+27);                // Total length: 10 bytes header + Payload + 2B CRC +15 bytes signature
    tl = (mav_len+22);                  // This works, the above does not!
    for (int i = (j+12); i < (tl+j); i++) {   
       if (i == (mav_len + 12)) {
        debug_serial_print("/");
      }
      if (i == (mav_len + 12 + 2+j)) {
        debug_serial_print("/");
      }
      PrintByte(bytes[i]); 
    }
    debug_serial_println();
  }

   debug_serial_print("Raw: ");
   for (int i = 0; i < 40; i++) {  //  unformatted
      PrintByte(bytes[i]); 
    }
   debug_serial_println();
  
}
//================================================================================================= 

void RB_To_Decode_To_SPort_and_GCS() {

  if (!MavRingBuff.isEmpty()) {
    noInterrupts();
    mavlink_message_t msg = (MavRingBuff.shift());  // Get a mavlink message from front of queue
    interrupts();
    #if defined Mav_Debug_RingBuff
  //   debug_print("Mavlink ring buffer R2Gmsg: ");  
  //    PrintMavBuffer(&R2Gmsg);
      debug_serial_print("Ring queue = "); debug_serial_println(String(MavRingBuff.size()));
    #endif
   
    DecodeOneMavFrame(msg);  // Decode a Mavlink frame from the ring buffer 

  }
                              //*** Decoded Mavlink to S.Port  ****

  if((millis() - em_millis) > 10) {   
    SPort_Blind_Inject_Packet();                // Emulate the sensor IDs received from XRS receiver on SPort
    em_millis=millis();
  }
}  

//================================================================================================= 

void MavToRingBuffer(mavlink_message_t * F2Rmsg) {

      // MAIN Queue
      if (MavRingBuff.isFull()) {
        debug_serial_println("MavRingBuff full. Dropping records!");
      }
       else {
        noInterrupts();
        MavRingBuff.push(*F2Rmsg);
        interrupts();
        #if defined Mav_Debug_RingBuff
          debug_print("Ring queue = "); 
          debug_println(String(MavRingBuff.size()));
        #endif
      }
  }
  
//================================================================================================= 

uint32_t bit32Extract(uint32_t dword,uint8_t displ, uint8_t lth); // Forward define

void DecodeOneMavFrame(mavlink_message_t R2Gmsg) {
  
   #if defined Mav_Print_All_Msgid
     uint16_t sz = sizeof(R2Gmsg);
     Debug.printf("FC to QGS - msgid = %3d Msg size =%3d\n",  R2Gmsg.msgid, sz);
   #endif

   switch(R2Gmsg.msgid) {
    
        case MAVLINK_MSG_ID_HEARTBEAT:    // #0   http://mavlink.org/messages/common
          ap_type_tmp = mavlink_msg_heartbeat_get_type(&R2Gmsg);   // Alex - don't contaminate the ap-type variable
          if (ap_type_tmp == 5 || ap_type_tmp == 6 || ap_type_tmp == 18 || ap_type_tmp == 26 || ap_type_tmp == 27 || ap_type_tmp == 30 || ap_type_tmp == 0) break;      
          // Ignore heartbeats from GCS (6) or Ant Trackers(5) or ADSB (27))
          ap_type = ap_type_tmp;
          ap_autopilot = mavlink_msg_heartbeat_get_autopilot(&R2Gmsg);
          ap_base_mode = mavlink_msg_heartbeat_get_base_mode(&R2Gmsg);
          ap_custom_mode = mavlink_msg_heartbeat_get_custom_mode(&R2Gmsg);
          
          px4_main_mode = bit32Extract(ap_custom_mode,16, 8);
          px4_sub_mode = bit32Extract(ap_custom_mode,24, 8);
          px4_flight_stack = (ap_autopilot == MAV_AUTOPILOT_PX4);
            
          if ((ap_base_mode >> 7) && (!homGood)) 
            MarkHome();  // If motors armed for the first time, then mark this spot as home

          PackSensorTable(0x5001, 0);

          #if defined Mav_Debug_All || defined Mav_Debug_FC_Heartbeat
            debug_serial_print("Mavlink from FC #0 Heartbeat: ");           
            debug_serial_print("ap_type="); debug_serial_print(ap_type);   
            debug_serial_print("  ap_autopilot="); debug_serial_print(ap_autopilot); 
            debug_serial_print("  ap_base_mode="); debug_serial_print(ap_base_mode); 
            debug_serial_print(" ap_custom_mode="); debug_serial_print(ap_custom_mode);

            if (px4_flight_stack) {         
              debug_serial_print(" px4_main_mode="); debug_serial_print(px4_main_mode); 
              debug_serial_print(" px4_sub_mode="); debug_serial_print(px4_sub_mode);  
              debug_serial_print(" ");debug_serial_print(PX4FlightModeName(px4_main_mode, px4_sub_mode));  
           }
            
            debug_serial_println();
          #endif

          
          break;
        case MAVLINK_MSG_ID_SYS_STATUS:   // #1
          if (R2Gmsg.compid != MAV_COMP_ID_AUTOPILOT1) return; // we are only interested in the autopilot status

          ap_onboard_control_sensors_health = mavlink_msg_sys_status_get_onboard_control_sensors_health(&R2Gmsg);
          ap_voltage_battery1= Get_Volt_Average1(mavlink_msg_sys_status_get_voltage_battery(&R2Gmsg));        // 1000 = 1V  i.e mV
          ap_current_battery1= Get_Current_Average1(mavlink_msg_sys_status_get_current_battery(&R2Gmsg));     //  100 = 1A, i.e dA
          if(ap_voltage_battery1> 21000) ap_ccell_count1= 6;
            else if (ap_voltage_battery1> 16800 && ap_ccell_count1!= 6) ap_ccell_count1= 5;
            else if(ap_voltage_battery1> 12600 && ap_ccell_count1!= 5) ap_ccell_count1= 4;
            else if(ap_voltage_battery1> 8400 && ap_ccell_count1!= 4) ap_ccell_count1= 3;
            else if(ap_voltage_battery1> 4200 && ap_ccell_count1!= 3) ap_ccell_count1= 2;
            else ap_ccell_count1= 0;
          
          #if defined Mav_Debug_All || defined Mav_Debug_SysStatus || defined Debug_Batteries
            debug_serial_print("Mavlink from FC #1 Sys_status: ");     
            debug_serial_print(" Sensor health=");
            debug_serial_print(ap_onboard_control_sensors_health);   // 32b bitwise 0: error, 1: healthy.
            debug_serial_print(" Bat volts=");
            debug_serial_print((float)ap_voltage_battery1/ 1000, 3);   // now V
            debug_serial_print("  Bat amps=");
            debug_serial_print((float)ap_current_battery1/ 100, 1);   // now A
              
            debug_serial_print("  mAh="); debug_serial_print(bat1.mAh, 6);    
            debug_serial_print("  Total mAh="); debug_serial_print(bat1.tot_mAh, 3);  // Consumed so far, calculated in Average module
         
            debug_serial_print("  Bat1 cell count= "); 
            debug_serial_println(ap_ccell_count1);
          #endif

          #if defined Send_Sensor_Health_Messages
          if ((millis() - health_millis) > 5000) {
            health_millis = millis();
            if ( bit32Extract(ap_onboard_control_sensors_health, 5, 1) ) {
              ap_severity = MAV_SEVERITY_CRITICAL;  // severity = 2
              strcpy(ap_text, "Bad GPS Health");
              PackMultipleTextChunks_5000(0x5000);
            } else
          
            if ( bit32Extract(ap_onboard_control_sensors_health, 0, 1) ) {
              ap_severity = MAV_SEVERITY_CRITICAL;  
              strcpy(ap_text, "Bad Gyro Health + 0x00 +0x00");
              PackMultipleTextChunks_5000(0x5000);
            } else 
                 
            if ( bit32Extract(ap_onboard_control_sensors_health, 1, 1) ) {
              ap_severity = MAV_SEVERITY_CRITICAL;  
              strcpy(ap_text, "Bad Accel Health");
              PackMultipleTextChunks_5000(0x5000);
            } else
          
            if ( bit32Extract(ap_onboard_control_sensors_health, 2, 1) ) {
              ap_severity = MAV_SEVERITY_CRITICAL;  
              strcpy(ap_text, "Bad Compass Health");
              PackMultipleTextChunks_5000(0x5000);
            } else
            
            if ( bit32Extract(ap_onboard_control_sensors_health, 3, 1) ) {
              ap_severity = MAV_SEVERITY_CRITICAL;  // severity = 2
              strcpy(ap_text, "Bad Baro Health");
              PackMultipleTextChunks_5000(0x5000);
            } else
          
            if ( bit32Extract(ap_onboard_control_sensors_health, 8, 1) ) {
              ap_severity = MAV_SEVERITY_CRITICAL;  
              strcpy(ap_text, "Bad LiDAR Health");
              PackMultipleTextChunks_5000(0x5000);
            } else 
                 
            if ( bit32Extract(ap_onboard_control_sensors_health, 6, 1) ) {
              ap_severity = MAV_SEVERITY_CRITICAL;  
              strcpy(ap_text, "Bad OptFlow Health + 0x00 +0x00");
              PackMultipleTextChunks_5000(0x5000);
            } else
          
            if ( bit32Extract(ap_onboard_control_sensors_health, 22, 1) ) {
             ap_severity = MAV_SEVERITY_CRITICAL;  
              strcpy(ap_text, "Bad or No Terrain Data");
              PackMultipleTextChunks_5000(0x5000);
            } else

            if ( bit32Extract(ap_onboard_control_sensors_health, 20, 1) ) {
             ap_severity = MAV_SEVERITY_CRITICAL;  
             strcpy(ap_text, "Geofence Breach");
             PackMultipleTextChunks_5000(0x5000);
           } else  
                 
            if ( bit32Extract(ap_onboard_control_sensors_health, 21, 1) ) {
              ap_severity = MAV_SEVERITY_CRITICAL;  
              strcpy(ap_text, "Bad AHRS");
              PackMultipleTextChunks_5000(0x5000);
            } else
          
           if ( bit32Extract(ap_onboard_control_sensors_health, 16, 1) ) {
             ap_severity = MAV_SEVERITY_CRITICAL;  
             strcpy(ap_text, "No RC Receiver");
             PackMultipleTextChunks_5000(0x5000);
           } else  

           if ( bit32Extract(ap_onboard_control_sensors_health, 24, 1) ) {
             ap_severity = MAV_SEVERITY_CRITICAL;  
             strcpy(ap_text, "Bad Logging");
             PackMultipleTextChunks_5000(0x5000);
           } 
         }                  
         #endif     
             
         PackSensorTable(0x5003, 0);
         
         break;              
        case MAVLINK_MSG_ID_PARAM_VALUE:          // #22
          len=mavlink_msg_param_value_get_param_id(&R2Gmsg, ap_param_id);
          ap_param_value=mavlink_msg_param_value_get_param_value(&R2Gmsg);
          ap_param_count=mavlink_msg_param_value_get_param_count(&R2Gmsg);
          ap_param_index=mavlink_msg_param_value_get_param_index(&R2Gmsg); 

          switch(ap_param_index) {      // if #define Battery_mAh_Source !=1 these will never arrive
            case 356:         // Bat1 Capacity
              ap_bat1_capacity = ap_param_value;
              #if defined Mav_Debug_All || defined Debug_Batteries
                debug_serial_print("Mavlink from FC #22 Param_Value: ");
                debug_serial_print("bat1 capacity=");
                debug_serial_println(ap_bat1_capacity);
              #endif
              break;
            case 364:         // Bat2 Capacity
              ap_bat2_capacity = ap_param_value;
              ap_bat_paramsRead = true;
              #if defined Mav_Debug_All || defined Debug_Batteries
                debug_serial_print("Mavlink from FC #22 Param_Value: ");
                debug_serial_print("bat2 capacity=");
                debug_serial_println(ap_bat2_capacity);
              #endif             
              break;
          } 
             
          #if defined Mav_Debug_All || defined Mav_Debug_Params || defined Mav_List_Params
            debug_serial_print("Mavlink from FC #22 Param_Value: ");
            debug_serial_print("param_id=");
            debug_serial_print(ap_param_id);
            debug_serial_print("  param_value=");
            debug_serial_print(ap_param_value, 4);
            debug_serial_print("  param_count=");
            debug_serial_print(ap_param_count);
            debug_serial_print("  param_index=");
            debug_serial_println(ap_param_index);
          #endif       
          break;    
        case MAVLINK_MSG_ID_GPS_RAW_INT:          // #24
          ap_fixtype = mavlink_msg_gps_raw_int_get_fix_type(&R2Gmsg);                   // 0 = No GPS, 1 =No Fix, 2 = 2D Fix, 3 = 3D Fix
          ap_sat_visible =  mavlink_msg_gps_raw_int_get_satellites_visible(&R2Gmsg);    // number of visible satellites
          if(ap_fixtype > 2)  {
            ap_lat24 = mavlink_msg_gps_raw_int_get_lat(&R2Gmsg);
            ap_lon24 = mavlink_msg_gps_raw_int_get_lon(&R2Gmsg);
            ap_amsl24 = mavlink_msg_gps_raw_int_get_alt(&R2Gmsg);                    // 1m =1000 
            ap_eph = mavlink_msg_gps_raw_int_get_eph(&R2Gmsg);                       // GPS HDOP 
            ap_cog = mavlink_msg_gps_raw_int_get_cog(&R2Gmsg);                       // Course over ground (NOT heading) in degrees * 100
     // mav2

           cur.lat =  (float)ap_lat24 / 1E7;
           cur.lon = (float)ap_lon24 / 1E7;
           cur.alt = ap_amsl24 / 1E3;
           
          }
          #if defined Mav_Debug_All || defined Mav_Debug_GPS_Raw
            debug_serial_print("Mavlink from FC #24 GPS_RAW_INT: ");  
            debug_serial_print("ap_fixtype="); debug_serial_print(ap_fixtype);
            if (ap_fixtype==0) debug_serial_print(" No GPS");
              else if (ap_fixtype==1) debug_serial_print(" No Fix");
              else if (ap_fixtype==2) debug_serial_print(" 2D Fix");
              else if (ap_fixtype==3) debug_serial_print(" 3D Fix");
              else if (ap_fixtype==4) debug_serial_print(" DGPS/SBAS aided");
              else if (ap_fixtype==5) debug_serial_print(" RTK Float");
              else if (ap_fixtype==6) debug_serial_print(" RTK Fixed");
              else if (ap_fixtype==7) debug_serial_print(" Static fixed");
              else if (ap_fixtype==8) debug_serial_print(" PPP");
              else debug_serial_print(" Unknown");

            debug_serial_print("  sats visible="); debug_serial_print(ap_sat_visible);
            debug_serial_print("  latitude="); debug_serial_print((float)(ap_lat24)/1E7, 7);
            debug_serial_print("  longitude="); debug_serial_print((float)(ap_lon24)/1E7, 7);
            debug_serial_print("  gps alt amsl="); debug_serial_print((float)(ap_amsl24)/1E3, 1);
            debug_serial_print("  eph (hdop)="); debug_serial_print((float)ap_eph);                 // HDOP
            debug_serial_print("  cog="); debug_serial_print((float)ap_cog / 100, 1);           // Course over ground in degrees
            //  mav2
            debug_serial_println();
          #endif 

           PackSensorTable(0x800, 0);   // 0x800 Lat
           PackSensorTable(0x800, 1);   // 0x800 Lon
           PackSensorTable(0x5002, 0);  // 0x5002 GPS Status
           PackSensorTable(0x5004, 0);  // 0x5004 Home         
              
          break;
          
        case MAVLINK_MSG_ID_RAW_IMU:   // #27
        #if defined Decode_Non_Essential_Mav
          ap27_xacc = mavlink_msg_raw_imu_get_xacc(&R2Gmsg);                 
          ap27_yacc = mavlink_msg_raw_imu_get_yacc(&R2Gmsg);
          ap27_zacc = mavlink_msg_raw_imu_get_zacc(&R2Gmsg);
          ap27_xgyro = mavlink_msg_raw_imu_get_xgyro(&R2Gmsg);                 
          ap27_ygyro = mavlink_msg_raw_imu_get_ygyro(&R2Gmsg);
          ap27_zgyro = mavlink_msg_raw_imu_get_zgyro(&R2Gmsg);
          ap27_xmag = mavlink_msg_raw_imu_get_xmag(&R2Gmsg);                 
          ap27_ymag = mavlink_msg_raw_imu_get_ymag(&R2Gmsg);
          ap27_zmag = mavlink_msg_raw_imu_get_zmag(&R2Gmsg);
          ap27_id = mavlink_msg_raw_imu_get_id(&R2Gmsg);         
          //  mav2
          ap26_temp = mavlink_msg_scaled_imu_get_temperature(&R2Gmsg);           
          #if defined Mav_Debug_All || defined Mav_Debug_Raw_IMU
            debug_serial_print("Mavlink from FC #27 Raw_IMU: ");
            debug_serial_print("accX="); debug_serial_print((float)ap27_xacc / 1000); 
            debug_serial_print("  accY="); debug_serial_print((float)ap27_yacc / 1000); 
            debug_serial_print("  accZ="); debug_serial_println((float)ap27_zacc / 1000);
            debug_serial_print("  xgyro="); debug_serial_print((float)ap27_xgyro / 1000, 3); 
            debug_serial_print("  ygyro="); debug_serial_print((float)ap27_ygyro / 1000, 3); 
            debug_serial_print("  zgyro="); debug_serial_print((float)ap27_zgyro / 1000, 3);
            debug_serial_print("  xmag="); debug_serial_print((float)ap27_xmag / 1000, 3); 
            debug_serial_print("  ymag="); debug_serial_print((float)ap27_ymag / 1000, 3); 
            debug_serial_print("  zmag="); debug_serial_print((float)ap27_zmag / 1000, 3);
            debug_serial_print("  id="); debug_serial_print((float)ap27_id);             
            debug_serial_print("  temp="); debug_serial_println((float)ap27_temp / 100, 2);    // cdegC               
          #endif 
        #endif             
          break; 
    
        case MAVLINK_MSG_ID_SCALED_PRESSURE:         // #29
        #if defined Decode_Non_Essential_Mav
          ap_press_abs = mavlink_msg_scaled_pressure_get_press_abs(&R2Gmsg);
          ap_temperature = mavlink_msg_scaled_pressure_get_temperature(&R2Gmsg);
          #if defined Mav_Debug_All || defined Mav_Debug_Scaled_Pressure
            debug_serial_print("Mavlink from FC #29 Scaled_Pressure: ");
            debug_serial_print("  press_abs=");  debug_serial_print(ap_press_abs,1);
            debug_serial_print("hPa  press_diff="); debug_serial_print(ap_press_diff, 3);
            debug_serial_print("hPa  temperature=");  debug_serial_print((float)(ap_temperature)/100, 1); 
            debug_serial_println("C");             
          #endif 
        #endif                          
          break;  
        case MAVLINK_MSG_ID_ATTITUDE:                // #30

          ap_roll = mavlink_msg_attitude_get_roll(&R2Gmsg);              // Roll angle (rad, -pi..+pi)
          ap_pitch = mavlink_msg_attitude_get_pitch(&R2Gmsg);            // Pitch angle (rad, -pi..+pi)
          ap_yaw = mavlink_msg_attitude_get_yaw(&R2Gmsg);                // Yaw angle (rad, -pi..+pi)

          ap_roll = RadToDeg(ap_roll);   // Now degrees
          ap_pitch = RadToDeg(ap_pitch);
          ap_yaw = RadToDeg(ap_yaw);
          
          #if defined Mav_Debug_All || defined Mav_Debug_Attitude   
            debug_serial_print("Mavlink from FC #30 Attitude: ");      
            debug_serial_print(" ap_roll degs=");
            debug_serial_print(ap_roll, 1);
            debug_serial_print(" ap_pitch degs=");   
            debug_serial_print(ap_pitch, 1);
            debug_serial_print(" ap_yaw degs=");         
            debug_serial_println(ap_yaw, 1);
          #endif             
      
          PackSensorTable(0x5006, 0 );  // 0x5006 Attitude      

          break;  
        case MAVLINK_MSG_ID_GLOBAL_POSITION_INT:     // #33
          if (ap_fixtype < 3) break;  
          ap_lat33 = mavlink_msg_global_position_int_get_lat(&R2Gmsg);             // Latitude, expressed as degrees * 1E7
          ap_lon33 = mavlink_msg_global_position_int_get_lon(&R2Gmsg);             // Pitch angle (rad, -pi..+pi)
          ap_amsl33 = mavlink_msg_global_position_int_get_alt(&R2Gmsg);          // Altitude above mean sea level (millimeters)
          ap_alt_ag = mavlink_msg_global_position_int_get_relative_alt(&R2Gmsg); // Altitude above ground (millimeters)
          ap_gps_hdg = mavlink_msg_global_position_int_get_hdg(&R2Gmsg);         // Vehicle heading (yaw angle) in degrees * 100, 0.0..359.99 degrees        
 
          cur.lat = (float)ap_lat33 / 1E7;
          cur.lon = (float)ap_lon33 / 1E7;
          cur.alt = ap_amsl33 / 1E3;
          cur.hdg = ap_gps_hdg / 100;

          #if defined Mav_Debug_All || defined Mav_Debug_GPS_Int
            debug_serial_print("Mavlink from FC #33 GPS Int: ");
            debug_serial_print(" ap_lat="); debug_serial_print((float)ap_lat33 / 1E7, 6);
            debug_serial_print(" ap_lon="); debug_serial_print((float)ap_lon33 / 1E7, 6);
            debug_serial_print(" ap_amsl="); debug_serial_print((float)ap_amsl33 / 1E3, 0);
            debug_serial_print(" ap_alt_ag="); debug_serial_print((float)ap_alt_ag / 1E3, 1);           
            debug_serial_print(" ap_gps_hdg="); debug_serial_println((float)ap_gps_hdg / 100, 1);
          #endif  
                
          break;  
        case MAVLINK_MSG_ID_SERVO_OUTPUT_RAW :          // #36
          
          #if defined PlusVersion
            PackSensorTable(0x50F1, 0);   // 0x50F1  SERVO_OUTPUT_RAW
          #endif  
                     
          break;  
        case MAVLINK_MSG_ID_MISSION_ITEM :          // #39
            ap_ms_seq = mavlink_msg_mission_item_get_seq(&R2Gmsg);
            ap_mission_type = mavlink_msg_mission_item_get_z(&R2Gmsg);                // MAV_MISSION_TYPE
                     
            #if defined Mav_Debug_All || defined Mav_Debug_Mission
              debug_serial_print("Mavlink from FC #39 Mission Item: ");
              debug_serial_print(" ap_ms_seq="); debug_serial_print(ap_ms_seq);  
              debug_serial_print(" ap_mission_type="); debug_serial_print(ap_mission_type); 
              debug_serial_println();    
            #endif
            
            if (ap_ms_seq > Max_Waypoints) {
              debug_serial_println(" Max Waypoints exceeded! Waypoint ignored.");
              break;
            }

             
          break;                    
        case MAVLINK_MSG_ID_MISSION_CURRENT:         // #42 should come down regularly as part of EXTENDED_status group
            ap_ms_seq =  mavlink_msg_mission_current_get_seq(&R2Gmsg);  
            
            #if defined Mav_Debug_All || defined Mav_Debug_Mission
            if (ap_ms_seq) {
              debug_serial_print("Mavlink from FC #42 Mission Current: ");
              debug_serial_print("ap_mission_current="); debug_serial_println(ap_ms_seq);   
            }
            #endif 
              
            if (ap_ms_seq > 0) ap_ms_current_flag = true;     //  Ok to send passthru frames 
  
          break; 
        case MAVLINK_MSG_ID_MISSION_COUNT :          // #44   received back after #43 Mission_Request_List sent
        #if defined Request_Missions_From_FC || defined Request_Mission_Count_From_FC
            ap_mission_count =  mavlink_msg_mission_count_get_count(&R2Gmsg); 
            #if defined Mav_Debug_All || defined Mav_Debug_Mission
              debug_serial_print("Mavlink from FC #44 Mission Count: ");
              debug_serial_print("ap_mission_count="); debug_serial_println(ap_mission_count);   
            #endif
            #if defined Request_Missions_From_FC
            if ((ap_mission_count > 0) && (ap_ms_count_ft)) {
              ap_ms_count_ft = false;
              RequestAllWaypoints(ap_mission_count);  // # multiple #40, then wait for them to arrive at #39
            }
            #endif
          break; 
        #endif
        
        case MAVLINK_MSG_ID_NAV_CONTROLLER_OUTPUT:   // #62
            ap_target_bearing = mavlink_msg_nav_controller_output_get_target_bearing(&R2Gmsg);  // Bearing to current waypoint/target
            ap_wp_dist = mavlink_msg_nav_controller_output_get_wp_dist(&R2Gmsg);                // Distance to active waypoint
            ap_xtrack_error = mavlink_msg_nav_controller_output_get_xtrack_error(&R2Gmsg);      // Current crosstrack error on x-y plane

            #if defined Mav_Debug_All || defined Mav_Debug_Waypoints
              debug_serial_print("Mavlink from FC #62 Nav_Controller_Output - (+Waypoint): ");
              debug_serial_print(" ap_target_bearing="); debug_serial_print(ap_target_bearing);   
              debug_serial_print(" ap_wp_dist="); debug_serial_print(ap_wp_dist);  
              debug_serial_print(" ap_xtrack_error="); debug_serial_print(ap_xtrack_error, 2);  
              debug_serial_println();    
            #endif
            
            #if defined PlusVersion  
              PackSensorTable(0x5009, 0);  // 0x5009 Waypoints  
            #endif       
        
          break;     
        case MAVLINK_MSG_ID_RC_CHANNELS:             // #65

            ap_chcnt = mavlink_msg_rc_channels_get_chancount(&R2Gmsg);
            ap_chan_raw[0] = mavlink_msg_rc_channels_get_chan1_raw(&R2Gmsg);   
            ap_chan_raw[1] = mavlink_msg_rc_channels_get_chan2_raw(&R2Gmsg);
            ap_chan_raw[2] = mavlink_msg_rc_channels_get_chan3_raw(&R2Gmsg);   
            ap_chan_raw[3] = mavlink_msg_rc_channels_get_chan4_raw(&R2Gmsg);  
            ap_chan_raw[4] = mavlink_msg_rc_channels_get_chan5_raw(&R2Gmsg);   
            ap_chan_raw[5] = mavlink_msg_rc_channels_get_chan6_raw(&R2Gmsg);
            ap_chan_raw[6] = mavlink_msg_rc_channels_get_chan7_raw(&R2Gmsg);   
            ap_chan_raw[7] = mavlink_msg_rc_channels_get_chan8_raw(&R2Gmsg);  
            ap_chan_raw[8] = mavlink_msg_rc_channels_get_chan9_raw(&R2Gmsg);   
            ap_chan_raw[9] = mavlink_msg_rc_channels_get_chan10_raw(&R2Gmsg);
            ap_chan_raw[10] = mavlink_msg_rc_channels_get_chan11_raw(&R2Gmsg);   
            ap_chan_raw[11] = mavlink_msg_rc_channels_get_chan12_raw(&R2Gmsg); 
            ap_chan_raw[12] = mavlink_msg_rc_channels_get_chan13_raw(&R2Gmsg);   
            ap_chan_raw[13] = mavlink_msg_rc_channels_get_chan14_raw(&R2Gmsg);
            ap_chan_raw[14] = mavlink_msg_rc_channels_get_chan15_raw(&R2Gmsg);   
            ap_chan_raw[15] = mavlink_msg_rc_channels_get_chan16_raw(&R2Gmsg);
            ap_chan_raw[16] = mavlink_msg_rc_channels_get_chan17_raw(&R2Gmsg);   
            ap_chan_raw[17] = mavlink_msg_rc_channels_get_chan18_raw(&R2Gmsg);
             
            #if defined Mav_Debug_All || defined Debug_Rssi || defined Mav_Debug_RC
              debug_serial_print("Mavlink from FC #65 RC_Channels: ");
              debug_serial_print("ap_chcnt="); debug_serial_print(ap_chcnt); 
              debug_serial_print(" PWM: ");
              for (int i=0 ; i < ap_chcnt ; i++) {
                debug_serial_print(" "); 
                debug_serial_print(i+1);
                debug_serial_print("=");  
                debug_serial_print(ap_chan_raw[i]);   
              }                         
            #endif             
          break;      
        case MAVLINK_MSG_ID_MISSION_ITEM_INT:       // #73   received back after #51 Mission_Request_Int sent
        #if defined Mav_Debug_All || defined Mav_Debug_Mission
          ap73_target_system =  mavlink_msg_mission_item_int_get_target_system(&R2Gmsg);   
          ap73_target_component =  mavlink_msg_mission_item_int_get_target_component(&R2Gmsg);   
          ap73_seq = mavlink_msg_mission_item_int_get_seq(&R2Gmsg);           // Waypoint ID (sequence number)
          ap73_frame = mavlink_msg_mission_item_int_get_frame(&R2Gmsg);       // MAV_FRAME The coordinate system of the waypoint.
          ap73_command = mavlink_msg_mission_item_int_get_command(&R2Gmsg);   // MAV_CMD The scheduled action for the waypoint.
          ap73_current = mavlink_msg_mission_item_int_get_current(&R2Gmsg);   // false:0, true:1
          ap73_autocontinue = mavlink_msg_mission_item_int_get_autocontinue(&R2Gmsg);   // Autocontinue to next waypoint
          ap73_param1 = mavlink_msg_mission_item_int_get_param1(&R2Gmsg);     // PARAM1, see MAV_CMD enum
          ap73_param2 = mavlink_msg_mission_item_int_get_param2(&R2Gmsg);     // PARAM2, see MAV_CMD enum
          ap73_param3 = mavlink_msg_mission_item_int_get_param3(&R2Gmsg);     // PARAM3, see MAV_CMD enum
          ap73_param4 = mavlink_msg_mission_item_int_get_param4(&R2Gmsg);     // PARAM4, see MAV_CMD enum
          ap73_x = mavlink_msg_mission_item_int_get_x(&R2Gmsg);               // PARAM5 / local: x position in meters * 1e4, global: latitude in degrees * 10^7
          ap73_y = mavlink_msg_mission_item_int_get_y(&R2Gmsg);               // PARAM6 / y position: local: x position in meters * 1e4, global: longitude in degrees *10^7
          ap73_z = mavlink_msg_mission_item_int_get_z(&R2Gmsg);               // PARAM7 / z position: global: altitude in meters (relative or absolute, depending on frame.
          ap73_mission_type = mavlink_msg_mission_item_int_get_mission_type(&R2Gmsg); // Mav2   MAV_MISSION_TYPE  Mission type.
 
          debug_serial_print("Mavlink from FC #73 Mission_Item_Int: ");
          debug_serial_print("target_system ="); debug_serial_print(ap73_target_system);   
          debug_serial_print("  target_component ="); debug_serial_print(ap73_target_component);   
          debug_serial_print(" _seq ="); debug_serial_print(ap73_seq);   
          debug_serial_print("  frame ="); debug_serial_print(ap73_frame);     
          debug_serial_print("  command ="); debug_serial_print(ap73_command);   
          debug_serial_print("  current ="); debug_serial_print(ap73_current);   
          debug_serial_print("  autocontinue ="); debug_serial_print(ap73_autocontinue);   
          debug_serial_print("  param1 ="); debug_serial_print(ap73_param1, 2); 
          debug_serial_print("  param2 ="); debug_serial_print(ap73_param2, 2); 
          debug_serial_print("  param3 ="); debug_serial_print(ap73_param3, 2); 
          debug_serial_print("  param4 ="); debug_serial_print(ap73_param4, 2);                                          
          debug_serial_print("  x ="); debug_serial_print(ap73_x);   
          debug_serial_print("  y ="); debug_serial_print(ap73_y);   
          debug_serial_print("  z ="); debug_serial_print(ap73_z, 4);    
          debug_serial_print("  mission_type ="); debug_serial_println(ap73_mission_type);                                                    

          break; 
        #endif
                               
        case MAVLINK_MSG_ID_VFR_HUD:                 //  #74
        {
          mavlink_vfr_hud_t param;
          mavlink_msg_vfr_hud_decode(&R2Gmsg, &param);

          // if ((param.airspeed == 0.0) ||
          //     (param.groundspeed == 0.0) ||
          //     ((param.heading < 0)||(param.heading > 360)) ||
          //     (param.throttle > 100) ||
          //     (param.alt == 0.0) ||
          //     (param.climb == 0.0))
          // {
          //   #if defined Mav_Debug_All || defined Mav_Debug_Hud
          //     Serial1.print("Mavlink from FC #74 VFR_HUD: ");
          //     Serial1.print("Airspeed= "); Serial1.print(ap_hud_air_spd_tmp, 2);                 // m/s    
          //     Serial1.print("  Groundspeed= "); Serial1.print(ap_hud_grd_spd_tmp, 2);            // m/s
          //     Serial1.print("  Heading= ");  Serial1.print(ap_hud_hdg_tmp);                      // deg
          //     Serial1.print("  Throttle %= ");  Serial1.print(ap_hud_throt_tmp);                 // %
          //     Serial1.print("  Baro alt= "); Serial1.print(ap_hud_bar_alt_tmp, 0);               // m                  
          //     Serial1.print("  Climb rate= "); Serial1.println(ap_hud_climb_tmp);                // m/s
          //   #endif 
          // } 
          // else 
          {
            ap_hud_air_spd = param.airspeed;
            ap_hud_grd_spd = param.groundspeed;      //  in m/s
            ap_hud_hdg = param.heading;              //  in degrees
            ap_hud_throt = param.throttle;           //  integer percent
            ap_hud_bar_alt = param.alt;              //  m
            ap_hud_climb = param.climb;              //  m/s

            cur.hdg = ap_hud_hdg;
            
          #if defined Mav_Debug_All || defined Mav_Debug_Hud
              debug_serial_print("Mavlink from FC #74 VFR_HUD: ");
              debug_serial_print("Airspeed= "); debug_serial_print(ap_hud_air_spd, 2);                 // m/s    
              debug_serial_print("  Groundspeed= "); debug_serial_print(ap_hud_grd_spd, 2);            // m/s
              debug_serial_print("  Heading= ");  debug_serial_print(ap_hud_hdg);                      // deg
              debug_serial_print("  Throttle %= ");  debug_serial_print(ap_hud_throt);                 // %
              debug_serial_print("  Baro alt= "); debug_serial_print(ap_hud_bar_alt, 0);               // m                  
              debug_serial_print("  Climb rate= "); debug_serial_println(ap_hud_climb);                // m/s
            #endif  

            PackSensorTable(0x5005, 0);  // 0x5005 VelYaw

            PackSensorTable(0x50F2, 0);  // 0x50F2 VFR HUD
          }            
          break; 
        }
        case MAVLINK_MSG_ID_RADIO_STATUS:         // #109
        {
            uint8_t ap_rssi109 = mavlink_msg_radio_status_get_rssi(&R2Gmsg);         // air signal strength
            if (!ap_rssi109) return; // this means the internal RFD900x is purely responding to heartbeats, but not linked to anything
              
            // If we get #109 then it must be a SiK fw radio, so use this record for rssi
            rs_millis = millis();  
            
            #if defined Rssi_In_Percent
              ap_rssi = ap_rssi109;          //  Percent
            #else
              ap_rssi = r900x_rssi_percentage(ap_rssi109);   // calculates link quality figure specifically for the RFD900x
            #endif
            
            #if defined Mav_Debug_All || defined Debug_Rssi || defined Mav_Debug_RC
              #ifndef RSSI_Override
                debug_serial_print("Auto RSSI_Source===>  ");
              #endif
            #endif     

            #if defined Mav_Debug_All || defined Debug_Radio_Status || defined Debug_Rssi
              debug_serial_print("Mavlink from FC #109 Radio: "); 
              debug_serial_print("ap_rssi109="); debug_serial_print(ap_rssi109);
              debug_serial_print("  remrssi="); debug_serial_print(ap_remrssi);
              debug_serial_print("  txbuf="); debug_serial_print(ap_txbuf);
              debug_serial_print("  noise="); debug_serial_print(ap_noise); 
              debug_serial_print("  remnoise="); debug_serial_print(ap_remnoise);
              debug_serial_print("  rxerrors="); debug_serial_print(ap_rxerrors);
              debug_serial_print("  fixed="); debug_serial_print(ap_fixed);  
            #endif 

          break;     
        }
        case MAVLINK_MSG_ID_POWER_STATUS:      // #125   https://mavlink.io/en/messages/common.html
          #if defined Decode_Non_Essential_Mav
            ap_Vcc = mavlink_msg_power_status_get_Vcc(&R2Gmsg);         // 5V rail voltage in millivolts
            ap_Vservo = mavlink_msg_power_status_get_Vservo(&R2Gmsg);   // servo rail voltage in millivolts
            ap_flags = mavlink_msg_power_status_get_flags(&R2Gmsg);     // power supply status flags (see MAV_POWER_status enum)
            #ifdef Mav_Debug_All
              debug_serial_print("Mavlink from FC #125 Power Status: ");
              debug_serial_print("Vcc= "); debug_serial_print(ap_Vcc); 
              debug_serial_print("  Vservo= ");  debug_serial_print(ap_Vservo);       
              debug_serial_print("  flags= ");  debug_serial_println(ap_flags);       
            #endif  
          #endif              
            break; 
         case MAVLINK_MSG_ID_BATTERY_STATUS:      // #147   https://mavlink.io/en/messages/common.html
          if (R2Gmsg.compid != MAV_COMP_ID_AUTOPILOT1) return; // we are only interested in the autopilot batteries
          ap_battery_id = mavlink_msg_battery_status_get_id(&R2Gmsg);  
          ap_current_battery = mavlink_msg_battery_status_get_current_battery(&R2Gmsg);      // in 10*milliamperes (1 = 10 milliampere)
          ap_current_consumed = mavlink_msg_battery_status_get_current_consumed(&R2Gmsg);    // mAh
          ap_battery_remaining = mavlink_msg_battery_status_get_battery_remaining(&R2Gmsg);  // (0%: 0, 100%: 100)  

          if (ap_battery_id == 0) {  // Battery 1
            fr_bat1_mAh = ap_current_consumed;                       
          } else if (ap_battery_id == 1) {  // Battery 2
            fr_bat2_mAh = ap_current_consumed;                              
          } 
             
          #if defined Mav_Debug_All || defined Debug_Batteries
            debug_serial_print("Mavlink from FC #147 Battery Status: ");
            debug_serial_print(" bat id= "); debug_serial_print(ap_battery_id); 
            debug_serial_print(" bat current mA= "); debug_serial_print(ap_current_battery*10); 
            debug_serial_print(" ap_current_consumed mAh= ");  debug_serial_print(ap_current_consumed);   
            if (ap_battery_id == 0) {
              debug_serial_print(" my di/dt mAh= ");  
              debug_serial_println(Total_mAh1(), 0);  
            }
            else {
              debug_serial_print(" my di/dt mAh= ");  
              debug_serial_println(Total_mAh2(), 0);   
            }    
        //  debug_serial_print(" bat % remaining= ");  debug_serial_println(ap_time_remaining);       
          #endif                        
          
          break;    
        case MAVLINK_MSG_ID_RADIO:             // #166   See #109 RADIO_status
        
          break; 
        case MAVLINK_MSG_ID_RANGEFINDER:       // #173   https://mavlink.io/en/messages/ardupilotmega.html
          ap_range = mavlink_msg_rangefinder_get_distance(&R2Gmsg);  // distance in meters

          #if defined Mav_Debug_All || defined Mav_Debug_Range
            debug_serial_print("Mavlink from FC #173 rangefinder: ");        
            debug_serial_print(" distance=");
            debug_serial_println(ap_range);   // now V
          #endif  

          PackSensorTable(0x5006, 0);  // 0x5006 Rangefinder
             
          break;            
        case MAVLINK_MSG_ID_BATTERY2:          // #181   https://mavlink.io/en/messages/ardupilotmega.html
          ap_voltage_battery2 = Get_Volt_Average2(mavlink_msg_battery2_get_voltage(&R2Gmsg));        // 1000 = 1V
          ap_current_battery2 = Get_Current_Average2(mavlink_msg_battery2_get_current_battery(&R2Gmsg));     //  100 = 1A
          if(ap_voltage_battery2 > 21000) ap_cell_count2 = 6;
            else if (ap_voltage_battery2 > 16800 && ap_cell_count2 != 6) ap_cell_count2 = 5;
            else if(ap_voltage_battery2 > 12600 && ap_cell_count2 != 5) ap_cell_count2 = 4;
            else if(ap_voltage_battery2 > 8400 && ap_cell_count2 != 4) ap_cell_count2 = 3;
            else if(ap_voltage_battery2 > 4200 && ap_cell_count2 != 3) ap_cell_count2 = 2;
            else ap_cell_count2 = 0;
   
          #if defined Mav_Debug_All || defined Debug_Batteries
            debug_serial_print("Mavlink from FC #181 Battery2: ");        
            debug_serial_print(" Bat volts=");
            debug_serial_print((float)ap_voltage_battery2 / 1000, 3);   // now V
            debug_serial_print("  Bat amps=");
            debug_serial_print((float)ap_current_battery2 / 100, 1);   // now A
              
            debug_serial_print("  mAh="); debug_serial_print(bat2.mAh, 6);    
            debug_serial_print("  Total mAh="); debug_serial_print(bat2.tot_mAh, 3);
         
            debug_serial_print("  Bat cell count= "); 
            debug_serial_println(ap_cell_count2);
          #endif

          PackSensorTable(0x5008, 0);   // 0x5008 Bat2       
                   
          break;
          
        case MAVLINK_MSG_ID_AHRS3:             // #182   https://mavlink.io/en/messages/ardupilotmega.html
          break;
        case MAVLINK_MSG_ID_STATUSTEXT:        // #253      
          ap_severity = mavlink_msg_statustext_get_severity(&R2Gmsg);
          len=mavlink_msg_statustext_get_text(&R2Gmsg, ap_text);

          #if defined Mav_Debug_All || defined Mav_Debug_StatusText
            debug_serial_print("Mavlink from FC #253 Statustext pushed onto MsgRingBuff: ");
            debug_serial_print(" Severity="); debug_serial_print(ap_severity);
            debug_serial_print(" "); debug_serial_print(MavSeverity(ap_severity));
            debug_serial_print("  Text= ");  debug_serial_print(" |"); debug_serial_print(ap_text); debug_serial_println("| ");
          #endif

          PackSensorTable(0x5000, 0);         // 0x5000 StatusText Message
          
          break;                                      
        default:
          #if defined Mav_Debug_All || defined Mav_Show_Unknown_Msgs
            debug_serial_print("Mavlink from FC: ");
            debug_serial_print("Unknown Message ID #");
            debug_serial_print(R2Gmsg.msgid);
            debug_serial_println(" Ignored"); 
          #endif

          break;
      }
}

//================================================================================================= 
void MarkHome()  {
  
  homGood = true;
  hom.lat = cur.lat;
  hom.lon = cur.lon;
  hom.alt = cur.alt;
  hom.hdg = cur.hdg;

  #if defined Mav_Debug_All || defined Mav_Debug_GPS_Int
    debug_serial_print("******************************************Mavlink in #33 GPS Int: Home established: ");       
    debug_serial_print("hom.lat=");  debug_serial_print(hom.lat, 7);
    debug_serial_print(" hom.lon=");  debug_serial_print(hom.lon, 7 );        
    debug_serial_print(" hom.alt="); debug_serial_print(hom.alt, 1);
    debug_serial_print(" hom.hdg="); debug_serial_println(hom.hdg);                   
 #endif  
}
//================================================================================================= 
#ifdef Request_Missions_From_FC
void RequestMission(uint16_t ms_seq) {    //  #40
  ap_sysid = 0xFF;
  ap_compid = 0xBE;
  ap_targsys = 1;
  ap_targcomp = 1; 
  ap_mission_type = 0;   // Mav2  0 = Items are mission commands for main mission
  
  mavlink_msg_mission_request_pack(ap_sysid, ap_compid, &G2Fmsg,
                               ap_targsys, ap_targcomp, ms_seq, ap_mission_type);

  Write_To_FC(40);
  #if defined Mav_Debug_All || defined Mav_Debug_Mission
    debug_serial_print("Mavlink to FC #40 Request Mission:  ms_seq="); debug_serial_println(ms_seq);
  #endif  
}
#endif 
 
//================================================================================================= 
#if defined Request_Missions_From_FC || defined Request_Mission_Count_From_FC
void RequestMissionList() {   // #43   get back #44 Mission_Count
  ap_sysid = 0xFF;
  ap_compid = 0xBE;
  ap_targsys = 1;
  ap_targcomp = 1; 
  ap_mission_type = 0;   // Mav2  0 = Items are mission commands for main mission
  
  mavlink_msg_mission_request_list_pack(ap_sysid, ap_compid, &G2Fmsg,
                               ap_targsys, ap_targcomp, ap_mission_type);
                             
  Write_To_FC(43);
  #if defined Mav_Debug_All || defined Mav_Debug_Mission
    debug_serial_println("Mavlink to FC #43 Request Mission List (count)");
  #endif  
}
#endif
//================================================================================================= 
#ifdef Request_Missions_From_FC
void RequestAllWaypoints(uint16_t ms_count) {
  for (int i = 0; i < ms_count; i++) {  //  Mission count = next empty WP, i.e. one too high
    RequestMission(i); 
  }
}
#endif
//================================================================================================= 
#ifdef Data_Streams_Enabled    
void RequestDataStreams() {    //  REQUEST_DATA_STREAM ( #66 ) DEPRECATED. USE SRx, SET_MESSAGE_INTERVAL INSTEAD

  ap_sysid = 0xFF;
  ap_compid = 0xBE;
  ap_targsys = 1;
  ap_targcomp = 1;

  const int maxStreams = 7;
  const uint8_t mavStreams[] = {
  MAV_DATA_STREAM_RAW_SENSORS,
  MAV_DATA_STREAM_EXTENDED_status,
  MAV_DATA_STREAM_RC_CHANNELS,
  MAV_DATA_STREAM_POSITION,
  MAV_DATA_STREAM_EXTRA1, 
  MAV_DATA_STREAM_EXTRA2,
  MAV_DATA_STREAM_EXTRA3
  };

  const uint16_t mavRates[] = { 0x04, 0x0a, 0x04, 0x0a, 0x04, 0x04, 0x04};
 // req_message_rate The requested interval between two messages of this type

  for (int i=0; i < maxStreams; i++) {
    mavlink_msg_request_data_stream_pack(ap_sysid, ap_compid, &G2Fmsg,
        ap_targsys, ap_targcomp, mavStreams[i], mavRates[i], 1);    // start_stop 1 to start sending, 0 to stop sending   
                          
  Write_To_FC(66);
    }
 // debug_serial_println("Mavlink to FC #66 Request Data Streams:");
}
#endif
//================================================================================================= 
//================================================================================================= 
//                                       F  R  S  K  Y  S  P  O  R  T
//================================================================================================= 
//================================================================================================= 
// Frsky variables     
short    crc;                         // of frsky-packet
uint8_t  time_slot_max = 16;              
uint32_t time_slot = 1;
float a, az, c, dis, dLat, dLon;
uint8_t sv_count = 0;

#if (defined TEENSY3X) // Teensy3x

  volatile uint8_t *uartC3;
  enum SPortMode { rx , tx };
  SPortMode mode, modeNow;

  void setSPortMode(SPortMode mode);  // Forward declaration

  void setSPortMode(SPortMode mode) {   

    if(mode == tx && modeNow !=tx) {
      *uartC3 |= 0x20;                 // Switch S.Port into send mode
      modeNow=mode;
      #ifdef Frs_Debug_All
      debug_serial_println("tx");
      #endif
    }
    else if(mode == rx && modeNow != rx) {   
      *uartC3 ^= 0x20;                 // Switch S.Port into receive mode
      modeNow=mode;
      #ifdef Frs_Debug_All
      debug_serial_println("rx");
      #endif
    }
  }
#endif

//=================================================================================================  
void SPort_Init(void)  {

  for (int i=0 ; i < sb_rows ; i++) {  // initialise sensor table
    sb[i].id = 0;
    sb[i].subid = 0;
    sb[i].millis = 0;
    sb[i].inuse = false;
  }

  int8_t frRx = -1;
  int8_t frTx = Fr_txPin;
  bool   frInvert = true;

  frSerial.begin(frBaud, SWSERIAL_8N1, frRx, frTx, frInvert);
  frSerial.enableIntTx(true);  
} 

//=================================================================================================  

void SPort_Blind_Inject_Packet() {
  #if (defined TEENSY3X)      
    setSPortMode(tx);
  #endif
 
  SPort_Inject_Packet();  


  // and back to main loop
}

//=================================================================================================  
void SPort_Inject_Packet() {
  #if defined Frs_Debug_Period
    ShowPeriod(0);   
  #endif  
       
  uint32_t fr_payload = 0; // Clear the payload field
    
  uint32_t sb_now = millis();
  int16_t sb_age;
  int16_t sb_subid_age;
  int16_t sb_max_tier1 = 0; 
  int16_t sb_max_tier2 = 0; 
  int16_t sb_max       = 0;     
  uint16_t ptr_tier1   = 0;                 // row with oldest sensor data
  uint16_t ptr_tier2   = 0; 
  uint16_t ptr         = 0; 

  // 2 tier scheduling. Tier 1 gets priority, tier2 (0x5000) only sent when tier 1 empty 
  
  // find the row with oldest sensor data = ptr 
  sb_unsent = 0;  // how many slots in-use

  uint16_t i = 0;
  while (i < sb_rows) {  
    
    if (sb[i].inuse) {
      sb_unsent++;   
      
      sb_age = (sb_now - sb[i].millis); 
      sb_subid_age = sb_age - sb[i].subid;  

      if (sb[i].id == 0x5000) {
        if (sb_subid_age >= sb_max_tier2) {
          sb_max_tier2 = sb_subid_age;
          ptr_tier2 = i;
        }
      } else {
      if (sb_subid_age >= sb_max_tier1) {
        sb_max_tier1 = sb_subid_age;
        ptr_tier1 = i;
        }   
      }
    } 
  i++;    
  } 
    
  if (sb_max_tier1 == 0) {            // if there are no tier 1 sensor entries
    if (sb_max_tier2 > 0) {           // but there are tier 2 entries
      ptr = ptr_tier2;                // send tier 2 instead
      sb_max = sb_max_tier2;
    }
  } else {
    ptr = ptr_tier1;                  // if there are tier1 entries send them
    sb_max = sb_max_tier1;
  }
  
  //debug_serial_println(sb_unsent);  // limited detriment :)  
        
  // send the packet if there is one
    if (sb_max > 0) {

     #ifdef Frs_Debug_Scheduler
       debug_serial_print(sb_unsent); 
       Debug.printf("\tPop  row= %3d", ptr );
       debug_serial_print("  id=");  debug_serial_print(sb[ptr].id, HEX);
       if (sb[ptr].id < 0x1000) debug_serial_print(" ");
       Debug.printf("  subid= %2d", sb[ptr].subid);       
       Debug.printf("  payload=%12d", sb[ptr].payload );
       Debug.printf("  age=%3d mS \n" , sb_max_tier1 );    
     #endif  
      
    if (sb[ptr].id == 0xF101) {
      SPort_SendByte(0x7E, false);   
      SPort_SendByte(0x1B, false);  
     }
                              
    SPort_SendDataFrame(0x1B, sb[ptr].id, sb[ptr].payload);
  
    sb[ptr].payload = 0;  
    sb[ptr].inuse = false; // free the row for re-use
  }
  
 }
//=================================================================================================  
void PushToEmptyRow(sb_t pter) {
  
  // find empty sensor row
  uint16_t j = 0;
  while (sb[j].inuse) {
    j++;
  }
  if (j >= sb_rows-1) {
    sens_buf_full_count++;
    if ( (sens_buf_full_count == 0) || (sens_buf_full_count%1000 == 0)) {
      debug_serial_println("Sensor buffer full. Check S.Port link");  // Report every so often
    }
    return;
  }
  
  sb_unsent++;
  
  #if defined Frs_Debug_Scheduler
    debug_serial_print(sb_unsent); 
    Debug.printf("\tPush row= %3d", j );
    debug_serial_print("  id="); debug_serial_print(pter.id, HEX);
    if (pter.id < 0x1000) debug_serial_print(" ");
    Debug.printf("  subid= %2d", pter.subid);    
    Debug.printf("  payload=%12d \n", pter.payload );
  #endif

  // The push
  pter.millis = millis();
  pter.inuse = true;
  sb[j] = pter;

}
//=================================================================================================  
void PackSensorTable(uint16_t id, uint8_t subid) {
  
  switch(id) {
    case 0x800:                  // data id 0x800 Lat & Lon
      if (subid == 0) {
        PackLat800(id);
      }
      if (subid == 1) {
        PackLon800(id);
      }
      break; 
           
    case 0x5000:                 // data id 0x5000 Status Text            
        PackMultipleTextChunks_5000(id);
        break;
        
    case 0x5001:                // data id 0x5001 AP Status
      Pack_AP_status_5001(id);
      break; 

    case 0x5002:                // data id 0x5002 GPS Status
      Pack_GPS_status_5002(id);
      break; 
          
    case 0x5003:                //data id 0x5003 Batt 1
      Pack_Bat1_5003(id);
      break; 
                    
    case 0x5004:                // data id 0x5004 Home
      Pack_Home_5004(id);
      break; 

    case 0x5005:                // data id 0x5005 Velocity and yaw
      Pack_VelYaw_5005(id);
      break; 

    case 0x5006:                // data id 0x5006 Attitude and range
      Pack_Atti_5006(id);
      break; 
      
    case 0x5007:                // data id 0x5007 Parameters 
      Pack_Parameters_5007(id);
      break; 
      
    case 0x5008:                // data id 0x5008 Batt 2
      Pack_Bat2_5008(id);
      break; 

    case 0x5009:                // data id 0x5009 Waypoints/Missions 
      Pack_WayPoint_5009(id);
      break;       

    case 0x50F1:                // data id 0x50F1 Servo_Raw            
      Pack_Servo_Raw_50F1(id);
      break;      

    case 0x50F2:                // data id 0x50F2 VFR HUD          
      Pack_VFR_Hud_50F2(id);
      break;    

    case 0x50F3:                // data id 0x50F3 Wind Estimate      
   //   Pack_Wind_Estimate_50F3(id);  // not presently implemented
      break; 
    case 0xF101:                // data id 0xF101 RSSI      
      Pack_Rssi_F101(id);      
      break;       
    default:
      debug_serial_print("Warning, sensor "); 
      debug_serial_print(String(id, HEX)); 
      debug_serial_println(" unknown");
      break;       
  }
              
}
//=================================================================================================  
void setSPortMode(SPortMode mode) {   
      if(mode == tx && modeNow !=tx) { 
        modeNow=mode;
        pb_rx = false;
        #if (defined ESP_Onewire) && (defined ESP32_SoftwareSerial)        
          frSerial.enableTx(true);  // Switch S.Port into send mode
        #endif
        #if defined Debug_SPort
          Debug.println("tx <======");
        #endif
      }   else 
      if(mode == rx && modeNow != rx) {   
        modeNow=mode; 
        pb_rx = true; 
        frSerial.enableTx(false);  // disable interrupts on tx pin
      } 
  }
//=================================================================================================  
  void frSerialSafeWrite(byte b) {
    setSPortMode(tx);
    frSerial.write(b);   
    #if defined Debug_SPort  
      PrintByte(b);
    #endif 
    delay(0); // yield to rtos for wifi & bt to get a sniff
  }
//=================================================================================================
void SPort_SendByte(uint8_t byte, bool addCrc) {
  #if (not defined inhibit_SPort) 
    
   if (!addCrc) {
      frSerialSafeWrite(byte);  
     return;       
   }

   CheckByteStuffAndSend(byte);
 
    // update CRC
    crc += byte;       //0-1FF
    crc += crc >> 8;   //0-100
    crc &= 0x00ff;
    crc += crc >> 8;   //0-0FF
    crc &= 0x00ff;
    
  #endif    
}
//=================================================================================================  
void CheckByteStuffAndSend(uint8_t byte) {
  #if (not defined inhibit_SPort) 
   if (byte == 0x7E) {
     frSerial.write(0x7D);
     frSerial.write(0x5E);
   } else if (byte == 0x7D) {
     frSerial.write(0x7D);
     frSerial.write(0x5D);  
   } else {
     frSerial.write(byte);
     }
  #endif     
}
//=================================================================================================  
void SPort_SendCrc() {
  uint8_t byte;
  byte = 0xFF-crc;

 CheckByteStuffAndSend(byte);
 
 // DisplayByte(byte);
 // debug_serial_println("");
  crc = 0;          // CRC reset
}
//=================================================================================================  
void SPort_SendDataFrame(uint8_t Instance, uint16_t Id, uint32_t value) {

  SPort_SendByte(0x7E, false);       //  START/STOP don't add into crc
  SPort_SendByte(Instance, false);   //  don't add into crc   
  SPort_SendByte(0x10, true );   //  Data framing byte
 
  uint8_t *bytes = (uint8_t*)&Id;
  #if defined Frs_Debug_Payload
    debug_serial_print("DataFrame. ID "); 
    DisplayByte(bytes[0]);
    debug_serial_print(" "); 
    DisplayByte(bytes[1]);
  #endif
  SPort_SendByte(bytes[0], true);
  SPort_SendByte(bytes[1], true);
  bytes = (uint8_t*)&value;
  SPort_SendByte(bytes[0], true);
  SPort_SendByte(bytes[1], true);
  SPort_SendByte(bytes[2], true);
  SPort_SendByte(bytes[3], true);
  
  #if defined Frs_Debug_Payload
    debug_serial_print("Payload (send order) "); 
    DisplayByte(bytes[0]);
    debug_serial_print(" "); 
    DisplayByte(bytes[1]);
    debug_serial_print(" "); 
    DisplayByte(bytes[2]);
    debug_serial_print(" "); 
    DisplayByte(bytes[3]);  
    debug_serial_print("Crc= "); 
    DisplayByte(0xFF-crc);
    debug_serial_println("/");  
  #endif
  
  SPort_SendCrc();

  delay(0); // yield to rtos for wifi & bt to get a sniff   
}
//=================================================================================================  
  uint32_t bit32Extract(uint32_t dword,uint8_t displ, uint8_t lth) {
  uint32_t r = (dword & createMask(displ,(displ+lth-1))) >> displ;
  return r;
}
//=================================================================================================  
// Mask then AND the shifted bits, then OR them to the payload
  void bit32Pack(uint32_t dword ,uint8_t displ, uint8_t lth, uint32_t * fr_payload) {   
  uint32_t dw_and_mask =  (dword<<displ) & (createMask(displ, displ+lth-1)); 
  *fr_payload |= dw_and_mask; 
}
//=================================================================================================  
  uint32_t bit32Unpack(uint32_t dword,uint8_t displ, uint8_t lth) {
  uint32_t r = (dword & createMask(displ,(displ+lth-1))) >> displ;
  return r;
}
//=================================================================================================  
uint32_t createMask(uint8_t lo, uint8_t hi) {
  uint32_t r = 0;
  for (unsigned i=lo; i<=hi; i++)
       r |= 1 << i;  
  return r;
}
//=================================================================================================  

void PackLat800(uint16_t id) {
  fr_gps_status = ap_fixtype < 3 ? ap_fixtype : 3;                   //  0 - 3 
  if (fr_gps_status < 3) return;
  if (px4_flight_stack) {
    fr_lat = Abs(ap_lat24) / 100 * 6;  // ap_lat * 60 / 1000
    if (ap_lat24<0) 
      ms2bits = 1;
    else ms2bits = 0;    
  } else {
    fr_lat = Abs(ap_lat33) / 100 * 6;  // ap_lat * 60 / 1000
    if (ap_lat33<0) 
      ms2bits = 1;
    else ms2bits = 0;
  }
  uint32_t fr_payload = 0;
  bit32Pack(fr_lat, 0, 30, &fr_payload);
  bit32Pack(ms2bits, 30, 2, &fr_payload);
          
  #if defined Frs_Debug_All || defined Frs_Debug_LatLon
    ShowPeriod(0); 
    debug_serial_print("FrSky in LatLon 0x800: ");
    debug_serial_print(" ap_lat33="); debug_serial_print((float)ap_lat33 / 1E7, 7); 
    debug_serial_print(" fr_lat="); debug_serial_print(fr_lat);  
    debug_serial_print(" fr_payload="); debug_serial_print(fr_payload); debug_serial_print(" ");
    DisplayPayload(fr_payload);
    int32_t r_lat = (bit32Unpack(fr_payload,0,30) * 100 / 6);
    debug_serial_print(" lat unpacked="); debug_serial_println(r_lat );    
  #endif

  sr.id = id;
  sr.subid = 0;  
  sr.payload = fr_payload;
  PushToEmptyRow(sr);        
}
//=================================================================================================  
void PackLon800(uint16_t id) { 
  fr_gps_status = ap_fixtype < 3 ? ap_fixtype : 3;                   //  0 - 3 
  if (fr_gps_status < 3) return;
  if (px4_flight_stack) {
    fr_lon = Abs(ap_lon24) / 100 * 6;  // ap_lon * 60 / 1000
    if (ap_lon24<0) {
      ms2bits = 3;
    }
    else {
      ms2bits = 2;    
    }
  } else {
    fr_lon = Abs(ap_lon33) / 100 * 6;  // ap_lon * 60 / 1000
    if (ap_lon33<0) { 
      ms2bits = 3;
    }
    else {
      ms2bits = 2;
    }
  }
  uint32_t fr_payload = 0;
  bit32Pack(fr_lon, 0, 30, &fr_payload);
  bit32Pack(ms2bits, 30, 2, &fr_payload);
          
  #if defined Frs_Debug_All || defined Frs_Debug_LatLon
    ShowPeriod(0); 
    debug_serial_print("FrSky in LatLon 0x800: ");  
    debug_serial_print(" ap_lon33="); debug_serial_print((float)ap_lon33 / 1E7, 7);     
    debug_serial_print(" fr_lon="); debug_serial_print(fr_lon); 
    debug_serial_print(" fr_payload="); debug_serial_print(fr_payload); debug_serial_print(" ");
    DisplayPayload(fr_payload);
    int32_t r_lon = (bit32Unpack(fr_payload,0,30) * 100 / 6);
    debug_serial_print(" lon unpacked="); debug_serial_println(r_lon );  
  #endif

  sr.id = id;
  sr.subid = 1;    
  sr.payload = fr_payload;
  PushToEmptyRow(sr); 
}
//=================================================================================================  
void PackMultipleTextChunks_5000(uint16_t id) {

  // status text  char[50] no null,  ap-text char[60]

  for (int i=0; i<50 ; i++) {       // Get text len
    if (ap_text[i]==0) {            // end of text
      len=i;
      break;
    }
  }
  
  ap_text[len+1]=0x00;
  ap_text[len+2]=0x00;  // mark the end of text chunk +
  ap_text[len+3]=0x00;
  ap_text[len+4]=0x00;
          
  ap_txtlth = len;
  
  // look for simple-mode status change messages       
  if (strcmp (ap_text,"SIMPLE mode on") == 0)
    ap_simple = true;
  else if (strcmp (ap_text,"SIMPLE mode off") == 0)
    ap_simple = false;

  fr_severity = ap_severity;
  fr_txtlth = ap_txtlth;
  memcpy(fr_text, ap_text, fr_txtlth+4);   // plus rest of last chunk at least
  fr_simple = ap_simple;

  #if defined Frs_Debug_All || defined Frs_Debug_StatusText
    ShowPeriod(0); 
    debug_serial_print("FrSky in AP_Text 0x5000: ");  
    debug_serial_print(" fr_severity="); debug_serial_print(fr_severity);
    debug_serial_print(" "); debug_serial_print(MavSeverity(fr_severity)); 
    debug_serial_print(" Text= ");  debug_serial_print(" |"); debug_serial_print(fr_text); debug_serial_println("| ");
  #endif

  fr_chunk_pntr = 0;

  while (fr_chunk_pntr <= (fr_txtlth)) {                 // send multiple 4 byte (32b) chunks
    
    fr_chunk_num = (fr_chunk_pntr / 4) + 1;
    
    fr_chunk[0] = fr_text[fr_chunk_pntr];
    fr_chunk[1] = fr_text[fr_chunk_pntr+1];
    fr_chunk[2] = fr_text[fr_chunk_pntr+2];
    fr_chunk[3] = fr_text[fr_chunk_pntr+3];
    
    uint32_t fr_payload = 0;
    bit32Pack(fr_chunk[0], 24, 7, &fr_payload);
    bit32Pack(fr_chunk[1], 16, 7, &fr_payload);
    bit32Pack(fr_chunk[2], 8, 7, &fr_payload);
    bit32Pack(fr_chunk[3], 0, 7, &fr_payload);
    
    #if defined Frs_Debug_All || defined Frs_Debug_StatusText
      ShowPeriod(0); 
      debug_serial_print(" fr_chunk_num="); debug_serial_print(fr_chunk_num); 
      debug_serial_print(" fr_txtlth="); debug_serial_print(fr_txtlth); 
      debug_serial_print(" fr_chunk_pntr="); debug_serial_print(fr_chunk_pntr); 
      debug_serial_print(" "); 
      strncpy(fr_chunk_print,fr_chunk, 4);
      fr_chunk_print[4] = 0x00;
      debug_serial_print(" |"); debug_serial_print(fr_chunk_print); debug_serial_print("| ");
      debug_serial_print(" fr_payload="); debug_serial_print(fr_payload); debug_serial_print(" ");
      DisplayPayload(fr_payload);
      debug_serial_println();
    #endif  

    if (fr_chunk_pntr+4 > (fr_txtlth)) {

      bit32Pack((fr_severity & 0x1), 7, 1, &fr_payload);            // ls bit of severity
      bit32Pack(((fr_severity & 0x2) >> 1), 15, 1, &fr_payload);    // mid bit of severity
      bit32Pack(((fr_severity & 0x4) >> 2) , 23, 1, &fr_payload);   // ms bit of severity                
      bit32Pack(0, 31, 1, &fr_payload);     // filler
      
      #if defined Frs_Debug_All || defined Frs_Debug_StatusText
        ShowPeriod(0); 
        debug_serial_print(" fr_chunk_num="); debug_serial_print(fr_chunk_num); 
        debug_serial_print(" fr_severity="); debug_serial_print(fr_severity);
        debug_serial_print(" "); debug_serial_print(MavSeverity(fr_severity)); 
        bool lsb = (fr_severity & 0x1);
        bool sb = (fr_severity & 0x2) >> 1;
        bool msb = (fr_severity & 0x4) >> 2;
        debug_serial_print(" ls bit="); debug_serial_print(lsb); 
        debug_serial_print(" mid bit="); debug_serial_print(sb); 
        debug_serial_print(" ms bit="); debug_serial_print(msb); 
        debug_serial_print(" fr_payload="); debug_serial_print(fr_payload); debug_serial_print(" ");
        DisplayPayload(fr_payload);
        debug_serial_println(); debug_serial_println();
     #endif 
     }

    sr.id = id;
    sr.subid = fr_chunk_num;
    sr.payload = fr_payload;
    PushToEmptyRow(sr); 

    #if defined Send_status_Text_3_Times 
      PushToEmptyRow(sr); 
      PushToEmptyRow(sr);
    #endif 
    
    fr_chunk_pntr +=4;
 }
  
  fr_chunk_pntr = 0;
   
}
//=================================================================================================  
void DisplayPayload(uint32_t pl)  {
  uint8_t *bytes;
  debug_serial_print("//");
  bytes = (uint8_t*)&pl;
  DisplayByte(bytes[3]);
  debug_serial_print(" "); 
  DisplayByte(bytes[2]);
  debug_serial_print(" "); 
  DisplayByte(bytes[1]);
  debug_serial_print(" "); 
  DisplayByte(bytes[0]);   
}
//=================================================================================================  
void Pack_AP_status_5001(uint16_t id) {
  if (ap_type == 6) return;      // If GCS heartbeat ignore it  -  yaapu  - ejs also handled at #0 read
  uint32_t fr_payload = 0;
 // fr_simple = ap_simple;         // Derived from "ALR SIMPLE mode on/off" text messages
  fr_simple = 0;  // stops repeated 'simple mode enabled' and flight mode messages
  fr_armed = ap_base_mode >> 7;  
  fr_land_complete = fr_armed;
  
  if (px4_flight_stack) 
    fr_flight_mode = PX4FlightModeNum(px4_main_mode, px4_sub_mode);
  else   //  APM Flight Stack
    fr_flight_mode = ap_custom_mode + 1; // AP_CONTROL_MODE_LIMIT - ls 5 bits

  fr_imu_temp = ap26_temp;
    
  
  bit32Pack(fr_flight_mode, 0, 5, &fr_payload);      // Flight mode   0-32 - 5 bits
  bit32Pack(fr_simple ,5, 2, &fr_payload);           // Simple/super simple mode flags
  bit32Pack(fr_land_complete ,7, 1, &fr_payload);    // Landed flag
  bit32Pack(fr_armed ,8, 1, &fr_payload);            // Armed
  bit32Pack(fr_bat_fs ,9, 1, &fr_payload);           // Battery failsafe flag
  bit32Pack(fr_ekf_fs ,10, 2, &fr_payload);          // EKF failsafe flag
  bit32Pack(px4_flight_stack ,12, 1, &fr_payload);   // px4_flight_stack flag
  bit32Pack(fr_imu_temp, 26, 6, &fr_payload);        // imu temperature in cdegC

  #if defined Frs_Debug_All || defined Frs_Debug_APStatus
    ShowPeriod(0); 
    debug_serial_print("FrSky in AP_status 0x5001: ");   
    debug_serial_print(" fr_flight_mode="); debug_serial_print(fr_flight_mode);
    debug_serial_print(" fr_simple="); debug_serial_print(fr_simple);
    debug_serial_print(" fr_land_complete="); debug_serial_print(fr_land_complete);
    debug_serial_print(" fr_armed="); debug_serial_print(fr_armed);
    debug_serial_print(" fr_bat_fs="); debug_serial_print(fr_bat_fs);
    debug_serial_print(" fr_ekf_fs="); debug_serial_print(fr_ekf_fs);
    debug_serial_print(" px4_flight_stack="); debug_serial_print(px4_flight_stack);
    debug_serial_print(" fr_imu_temp="); debug_serial_print(fr_imu_temp);
    debug_serial_print(" fr_payload="); debug_serial_print(fr_payload); debug_serial_print(" ");
    DisplayPayload(fr_payload);
    debug_serial_println();
  #endif

    sr.id = id;
    sr.subid = 0;
    sr.payload = fr_payload;
    PushToEmptyRow(sr);         
}
//=================================================================================================  
void Pack_GPS_status_5002(uint16_t id) {
  uint32_t fr_payload = 0;
  if (ap_sat_visible > 15)
    fr_numsats = 15;
  else
    fr_numsats = ap_sat_visible;
  
  bit32Pack(fr_numsats ,0, 4, &fr_payload); 
          
  fr_gps_status = ap_fixtype < 3 ? ap_fixtype : 3;                   //  0 - 3
  fr_gps_adv_status = ap_fixtype > 3 ? ap_fixtype - 3 : 0;           //  4 - 8 -> 0 - 3   
          
  fr_amsl = ap_amsl24 / 100;  // dm
  fr_hdop = ap_eph /10;
          
  bit32Pack(fr_gps_status ,4, 2, &fr_payload);       // part a, 3 bits
  bit32Pack(fr_gps_adv_status ,14, 2, &fr_payload);  // part b, 3 bits
          
  #if defined Frs_Debug_All || defined Frs_Debug_GPS_status
    ShowPeriod(0); 
    debug_serial_print("FrSky in GPS Status 0x5002: ");   
    debug_serial_print(" fr_numsats="); debug_serial_print(fr_numsats);
    debug_serial_print(" fr_gps_status="); debug_serial_print(fr_gps_status);
    debug_serial_print(" fr_gps_adv_status="); debug_serial_print(fr_gps_adv_status);
    debug_serial_print(" fr_amsl="); debug_serial_print(fr_amsl);
    debug_serial_print(" fr_hdop="); debug_serial_print(fr_hdop);
  #endif
          
  fr_amsl = prep_number(fr_amsl,2,2);                       // Must include exponent and mantissa    
  fr_hdop = prep_number(fr_hdop,2,1);
          
  #if defined Frs_Debug_All || defined Frs_Debug_GPS_status
    debug_serial_print(" After prep: fr_amsl="); debug_serial_print(fr_amsl);
    debug_serial_print(" fr_hdop="); debug_serial_print(fr_hdop); 
    debug_serial_print(" fr_payload="); debug_serial_print(fr_payload); debug_serial_print(" ");
    DisplayPayload(fr_payload);
    debug_serial_println(); 
  #endif     
              
  bit32Pack(fr_hdop ,6, 8, &fr_payload);
  bit32Pack(fr_amsl ,22, 9, &fr_payload);
  bit32Pack(0, 31,0, &fr_payload);  // 1=negative 

  sr.id = id;
  sr.subid = 0;
  sr.payload = fr_payload;
  PushToEmptyRow(sr); 
}
//=================================================================================================  
void Pack_Bat1_5003(uint16_t id) {   //  Into sensor table from #1 SYS_status only
  uint32_t fr_payload = 0;
  fr_bat1_volts = ap_voltage_battery1 / 100;         // Were mV, now dV  - V * 10
  fr_bat1_amps = ap_current_battery1 ;               // Remain       dA  - A * 10   
  
  // fr_bat1_mAh is populated at #147 depending on battery id.  Into sensor table from #1 SYS_status only.
  //fr_bat1_mAh = Total_mAh1();  // If record type #147 is not sent and good
  
  #if defined Frs_Debug_All || defined Debug_Batteries
    ShowPeriod(0); 
    debug_serial_print("FrSky in Bat1 0x5003: ");   
    debug_serial_print(" fr_bat1_volts="); debug_serial_print(fr_bat1_volts);
    debug_serial_print(" fr_bat1_amps="); debug_serial_print(fr_bat1_amps);
    debug_serial_print(" fr_bat1_mAh="); debug_serial_print(fr_bat1_mAh);
    debug_serial_print(" fr_payload="); debug_serial_print(fr_payload); debug_serial_print(" ");
    DisplayPayload(fr_payload);
    debug_serial_println();               
  #endif
          
  bit32Pack(fr_bat1_volts ,0, 9, &fr_payload);
  fr_bat1_amps = prep_number(roundf(fr_bat1_amps * 0.1F),2,1);          
  bit32Pack(fr_bat1_amps,9, 8, &fr_payload);
  bit32Pack(fr_bat1_mAh,17, 15, &fr_payload);

  sr.id = id;
  sr.subid = 0;
  sr.payload = fr_payload;
  PushToEmptyRow(sr); 
                       
}
//=================================================================================================  
void Pack_Home_5004(uint16_t id) {
    uint32_t fr_payload = 0;
    
    lon1=hom.lon/180*PI;  // degrees to radians
    lat1=hom.lat/180*PI;
    lon2=cur.lon/180*PI;
    lat2=cur.lat/180*PI;

    //Calculate azimuth bearing of craft from home
    a=atan2(sin(lon2-lon1)*cos(lat2), cos(lat1)*sin(lat2)-sin(lat1)*cos(lat2)*cos(lon2-lon1));
    az=a*180/PI;  // radians to degrees
    if (az<0) az=360+az;

    fr_home_angle = Add360(az, -180);                           // Is now the angle from the craft to home in degrees
  
    fr_home_arrow = fr_home_angle * 0.3333;                     // Units of 3 degrees

    // Calculate the distance from home to craft
    dLat = (lat2-lat1);
    dLon = (lon2-lon1);
    a = sin(dLat/2) * sin(dLat/2) + sin(dLon/2) * sin(dLon/2) * cos(lat1) * cos(lat2); 
    c = 2* asin(sqrt(a));    // proportion of Earth's radius
    dis = 6371000 * c;       // radius of the Earth is 6371km

    if (homGood)
      fr_home_dist = (int)dis;
    else
      fr_home_dist = 0;

      fr_home_alt = ap_alt_ag / 100;    // mm->dm
        
   #if defined Frs_Debug_All || defined Frs_Debug_Home
     ShowPeriod(0); 
     debug_serial_print("FrSky in Home 0x5004: ");         
     debug_serial_print("fr_home_dist=");  debug_serial_print(fr_home_dist);
     debug_serial_print(" fr_home_alt=");  debug_serial_print(fr_home_alt);
     debug_serial_print(" az=");  debug_serial_print(az);
     debug_serial_print(" fr_home_angle="); debug_serial_print(fr_home_angle);  
     debug_serial_print(" fr_home_arrow="); debug_serial_print(fr_home_arrow);         // units of 3 deg  
     debug_serial_print(" fr_payload="); debug_serial_print(fr_payload); debug_serial_print(" ");
     DisplayPayload(fr_payload);
    debug_serial_println();      
   #endif
   fr_home_dist = prep_number(roundf(fr_home_dist), 3, 2);
   bit32Pack(fr_home_dist ,0, 12, &fr_payload);
   fr_home_alt = prep_number(roundf(fr_home_alt), 3, 2);
   bit32Pack(fr_home_alt ,12, 12, &fr_payload);
   if (fr_home_alt < 0)
     bit32Pack(1,24, 1, &fr_payload);
   else  
     bit32Pack(0,24, 1, &fr_payload);
   bit32Pack(fr_home_arrow,25, 7, &fr_payload);

   sr.id = id;
   sr.subid = 0;
   sr.payload = fr_payload;
   PushToEmptyRow(sr); 

}

//=================================================================================================  
void Pack_VelYaw_5005(uint16_t id) {
  uint32_t fr_payload = 0;
  
  fr_vy = ap_hud_climb * 10;   // from #74   m/s to dm/s;
  fr_vx = ap_hud_grd_spd * 10;  // from #74  m/s to dm/s

  //fr_yaw = (float)ap_gps_hdg / 10;  // (degrees*100) -> (degrees*10)
  fr_yaw = ap_hud_hdg * 10;              // degrees -> (degrees*10)
  
  #if defined Frs_Debug_All || defined Frs_Debug_YelYaw
    ShowPeriod(0); 
    debug_serial_print("FrSky in VelYaw 0x5005:");  
    debug_serial_print(" fr_vy=");  debug_serial_print(fr_vy);       
    debug_serial_print(" fr_vx=");  debug_serial_print(fr_vx);
    debug_serial_print(" fr_yaw="); debug_serial_print(fr_yaw);
     
  #endif
  if (fr_vy<0)
    bit32Pack(1, 8, 1, &fr_payload);
  else
    bit32Pack(0, 8, 1, &fr_payload);
  fr_vy = prep_number(roundf(fr_vy), 2, 1);  // Vertical velocity
  bit32Pack(fr_vy, 0, 8, &fr_payload);   

  fr_vx = prep_number(roundf(fr_vx), 2, 1);  // Horizontal velocity
  bit32Pack(fr_vx, 9, 8, &fr_payload);    
  fr_yaw = fr_yaw * 0.5f;                   // Unit = 0.2 deg
  bit32Pack(fr_yaw ,17, 11, &fr_payload);  

 #if defined Frs_Debug_All || defined Frs_Debug_YelYaw
   debug_serial_print(" After prep:"); 
   debug_serial_print(" fr_vy=");  debug_serial_print((int)fr_vy);          
   debug_serial_print(" fr_vx=");  debug_serial_print((int)fr_vx);  
   debug_serial_print(" fr_yaw="); debug_serial_print((int)fr_yaw);  
   debug_serial_print(" fr_payload="); debug_serial_print(fr_payload); debug_serial_print(" ");
   DisplayPayload(fr_payload);
   debug_serial_println();                 
 #endif

 sr.id = id;
 sr.subid = 0;
 sr.payload = fr_payload;
 PushToEmptyRow(sr); 
    
}
//=================================================================================================   
void Pack_Atti_5006(uint16_t id) {
  uint32_t fr_payload = 0;
  
  fr_roll = (ap_roll * 5) + 900;             //  -- fr_roll units = [0,1800] ==> [-180,180]
  fr_pitch = (ap_pitch * 5) + 450;           //  -- fr_pitch units = [0,900] ==> [-90,90]
  fr_range = roundf(ap_range*100);   
  bit32Pack(fr_roll, 0, 11, &fr_payload);
  bit32Pack(fr_pitch, 11, 10, &fr_payload); 
  bit32Pack(prep_number(fr_range,3,1), 21, 11, &fr_payload);
  #if defined Frs_Debug_All || defined Frs_Debug_Attitude
    ShowPeriod(0); 
    debug_serial_print("FrSky in Attitude 0x5006: ");         
    debug_serial_print("fr_roll=");  debug_serial_print(fr_roll);
    debug_serial_print(" fr_pitch=");  debug_serial_print(fr_pitch);
    debug_serial_print(" fr_range="); debug_serial_print(fr_range);
    debug_serial_print(" Frs_Attitude Payload="); debug_serial_print(fr_payload);  
  #endif

  sr.id = id;
  sr.subid = 0;
  sr.payload = fr_payload;
  PushToEmptyRow(sr);  
     
}
//=================================================================================================  
void Pack_Parameters_5007(uint16_t id) {

  uint32_t fr_payload = 0;
  app_count++;
    
  switch(app_count) {
    case 1:                                    // Frame type
      fr_param_id = 1;
      fr_frame_type = ap_type;
      
      bit32Pack(fr_frame_type, 0, 24, &fr_payload);
      bit32Pack(fr_param_id, 24, 4, &fr_payload);

      #if defined Frs_Debug_All || defined Frs_Debug_Params
        ShowPeriod(0);  
        debug_serial_print("Frsky out Params 0x5007: ");   
        debug_serial_print(" fr_param_id="); debug_serial_print(fr_param_id);
        debug_serial_print(" fr_frame_type="); debug_serial_print(fr_frame_type);  
        debug_serial_print(" fr_payload="); debug_serial_print(fr_payload);  debug_serial_print(" "); 
        DisplayPayload(fr_payload);
        debug_serial_println();                
      #endif
      
      sr.id = id;     
      sr.subid = 1;
      sr.payload = fr_payload;
      PushToEmptyRow(sr);
      break;    
       
    case 2:    // Battery pack 1 capacity
      fr_param_id = 4;
      #if (Battery_mAh_Source == 2)    // Local
        fr_bat1_capacity = getWorld()->getParameters()->getBattCapacitymAh();
      #elif  (Battery_mAh_Source == 1) //  FC
        fr_bat1_capacity = ap_bat1_capacity;
      #endif 

      bit32Pack(fr_bat1_capacity, 0, 24, &fr_payload);
      bit32Pack(fr_param_id, 24, 4, &fr_payload);

      #if defined Frs_Debug_All || defined Frs_Debug_Params || defined Debug_Batteries
        ShowPeriod(0);       
        debug_serial_print("Frsky out Params 0x5007: ");   
        debug_serial_print(" fr_param_id="); debug_serial_print(fr_param_id);
        debug_serial_print(" fr_bat1_capacity="); debug_serial_print(fr_bat1_capacity);  
        debug_serial_print(" fr_payload="); debug_serial_print(fr_payload);  debug_serial_println(" "); 
        DisplayPayload(fr_payload);
        debug_serial_println();                   
      #endif
      
      sr.id = id;
      sr.subid = 4;
      sr.payload = fr_payload;
      PushToEmptyRow(sr);
      break; 
      
    case 3:                 // Battery pack 2 capacity
      fr_param_id = 5;
      #if (Battery_mAh_Source == 2)    // Local
        fr_bat2_capacity = getWorld()->getParameters()->getBat2CapacitymAh();
      #elif  (Battery_mAh_Source == 1) //  FC
        fr_bat2_capacity = ap_bat2_capacity;
      #endif  

      bit32Pack(fr_bat2_capacity, 0, 24, &fr_payload);
      bit32Pack(fr_param_id, 24, 4, &fr_payload);
      
      #if defined Frs_Debug_All || defined Frs_Debug_Params || defined Debug_Batteries
        ShowPeriod(0);  
        debug_serial_print("Frsky out Params 0x5007: ");   
        debug_serial_print(" fr_param_id="); debug_serial_print(fr_param_id);
        debug_serial_print(" fr_bat2_capacity="); debug_serial_print(fr_bat2_capacity); 
        debug_serial_print(" fr_payload="); debug_serial_print(fr_payload);  debug_serial_print(" "); 
        DisplayPayload(fr_payload);
        debug_serial_println();           
      #endif
      
      sr.subid = 5;
      sr.payload = fr_payload;
      PushToEmptyRow(sr); 
      break; 
    
     case 4:               // Number of waypoints in mission                       
      fr_param_id = 6;
      fr_mission_count = ap_mission_count;

      bit32Pack(fr_mission_count, 0, 24, &fr_payload);
      bit32Pack(fr_param_id, 24, 4, &fr_payload);

      sr.id = id;
      sr.subid = 6;
      PushToEmptyRow(sr);       
      
      #if defined Frs_Debug_All || defined Frs_Debug_Params || defined Debug_Batteries
        ShowPeriod(0); 
        debug_serial_print("Frsky out Params 0x5007: ");   
        debug_serial_print(" fr_param_id="); debug_serial_print(fr_param_id);
        debug_serial_print(" fr_mission_count="); debug_serial_println(fr_mission_count);           
      #endif
 
      fr_paramsSent = true;          // get this done early on and then regularly thereafter
      app_count = 0;
      break;
  }    
}
//=================================================================================================  
void Pack_Bat2_5008(uint16_t id) {
   uint32_t fr_payload = 0;
   
   fr_bat2_volts = ap_voltage_battery2 / 100;         // Were mV, now dV  - V * 10
   fr_bat2_amps = ap_current_battery2 ;               // Remain       dA  - A * 10   
   
  // fr_bat2_mAh is populated at #147 depending on battery id
  //fr_bat2_mAh = Total_mAh2();  // If record type #147 is not sent and good
  
  #if defined Frs_Debug_All || defined Debug_Batteries
    ShowPeriod(0);  
    debug_serial_print("FrSky in Bat2 0x5008: ");   
    debug_serial_print(" fr_bat2_volts="); debug_serial_print(fr_bat2_volts);
    debug_serial_print(" fr_bat2_amps="); debug_serial_print(fr_bat2_amps);
    debug_serial_print(" fr_bat2_mAh="); debug_serial_print(fr_bat2_mAh);
    debug_serial_print(" fr_payload="); debug_serial_print(fr_payload);  debug_serial_print(" "); 
    DisplayPayload(fr_payload);
    debug_serial_println();                  
  #endif        
          
  bit32Pack(fr_bat2_volts ,0, 9, &fr_payload);
  fr_bat2_amps = prep_number(roundf(fr_bat2_amps * 0.1F),2,1);          
  bit32Pack(fr_bat2_amps,9, 8, &fr_payload);
  bit32Pack(fr_bat2_mAh,17, 15, &fr_payload);      

  sr.id = id;
  sr.subid = 1;
  sr.payload = fr_payload;
  PushToEmptyRow(sr);          
}


//=================================================================================================  

void Pack_WayPoint_5009(uint16_t id) {
  uint32_t fr_payload = 0;
  
  fr_ms_seq = ap_ms_seq;                                      // Current WP seq number, wp[0] = wp1, from regular #42
  
  fr_ms_dist = ap_wp_dist;                                        // Distance to next WP  

  fr_ms_xtrack = ap_xtrack_error;                                 // Cross track error in metres from #62
  fr_ms_target_bearing = ap_target_bearing;                       // Direction of next WP
  fr_ms_cog = ap_cog * 0.01;                                      // COG in degrees from #24
  int32_t angle = (int32_t)wrap_360(fr_ms_target_bearing - fr_ms_cog);
  int32_t arrowStep = 360 / 8; 
  fr_ms_offset = ((angle + (arrowStep/2)) / arrowStep) % 8;       // Next WP bearing offset from COG

  /*
   
0 - up
1 - up-right
2 - right
3 - down-right
4 - down
5 - down - left
6 - left
7 - up - left
 
   */
  #if defined Frs_Debug_All || defined Frs_Debug_Mission
    ShowPeriod(0);  
    debug_serial_print("FrSky in RC 0x5009: ");   
    debug_serial_print(" fr_ms_seq="); debug_serial_print(fr_ms_seq);
    debug_serial_print(" fr_ms_dist="); debug_serial_print(fr_ms_dist);
    debug_serial_print(" fr_ms_xtrack="); debug_serial_print(fr_ms_xtrack, 3);
    debug_serial_print(" fr_ms_target_bearing="); debug_serial_print(fr_ms_target_bearing, 0);
    debug_serial_print(" fr_ms_cog="); debug_serial_print(fr_ms_cog, 0);  
    debug_serial_print(" fr_ms_offset="); debug_serial_print(fr_ms_offset);
    debug_serial_print(" fr_payload="); debug_serial_print(fr_payload);  debug_serial_print(" "); 
    DisplayPayload(fr_payload);         
    debug_serial_println();      
  #endif

  bit32Pack(fr_ms_seq, 0, 10, &fr_payload);    //  WP number

  fr_ms_dist = prep_number(roundf(fr_ms_dist), 3, 2);       //  number, digits, power
  bit32Pack(fr_ms_dist, 10, 12, &fr_payload);    

  fr_ms_xtrack = prep_number(roundf(fr_ms_xtrack), 1, 1);  
  bit32Pack(fr_ms_xtrack, 22, 6, &fr_payload); 

  bit32Pack(fr_ms_offset, 29, 3, &fr_payload);  

  sr.id = id;
  sr.subid = 1;
  sr.payload = fr_payload;
  PushToEmptyRow(sr); 
        
}

//=================================================================================================  
void Pack_Servo_Raw_50F1(uint16_t id) {
uint8_t sv_chcnt = 8;
  uint32_t fr_payload = 0;
  
  if (sv_count+4 > sv_chcnt) { // 4 channels at a time
    sv_count = 0;
    return;
  } 

  uint8_t  chunk = sv_count / 4; 

  fr_sv[1] = PWM_To_63(ap_chan_raw[sv_count]);     // PWM 1000 to 2000 -> 6bit 0 to 63
  fr_sv[2] = PWM_To_63(ap_chan_raw[sv_count+1]);    
  fr_sv[3] = PWM_To_63(ap_chan_raw[sv_count+2]); 
  fr_sv[4] = PWM_To_63(ap_chan_raw[sv_count+3]); 

  bit32Pack(chunk, 0, 4, &fr_payload);                // chunk number, 0 = chans 1-4, 1=chans 5-8, 2 = chans 9-12, 3 = chans 13 -16 .....
  bit32Pack(Abs(fr_sv[1]) ,4, 6, &fr_payload);        // fragment 1 
  if (fr_sv[1] < 0)
    bit32Pack(1, 10, 1, &fr_payload);                 // neg
  else 
    bit32Pack(0, 10, 1, &fr_payload);                 // pos          
  bit32Pack(Abs(fr_sv[2]), 11, 6, &fr_payload);      // fragment 2 
  if (fr_sv[2] < 0) 
    bit32Pack(1, 17, 1, &fr_payload);                 // neg
  else 
    bit32Pack(0, 17, 1, &fr_payload);                 // pos   
  bit32Pack(Abs(fr_sv[3]), 18, 6, &fr_payload);       // fragment 3
  if (fr_sv[3] < 0)
    bit32Pack(1, 24, 1, &fr_payload);                 // neg
  else 
    bit32Pack(0, 24, 1, &fr_payload);                 // pos      
  bit32Pack(Abs(fr_sv[4]), 25, 6, &fr_payload);       // fragment 4 
  if (fr_sv[4] < 0)
    bit32Pack(1, 31, 1, &fr_payload);                 // neg
  else 
    bit32Pack(0, 31, 1, &fr_payload);                 // pos  
        
  uint8_t sv_num = sv_count % 4;

  sr.id = id;
  sr.subid = sv_num + 1;
  sr.payload = fr_payload;
  PushToEmptyRow(sr); 

  #if defined Frs_Debug_All || defined Frs_Debug_Servo
    ShowPeriod(0);  
    debug_serial_print("FrSky in Servo_Raw 0x50F1: ");  
    debug_serial_print(" sv_chcnt="); debug_serial_print(sv_chcnt); 
    debug_serial_print(" sv_count="); debug_serial_print(sv_count); 
    debug_serial_print(" chunk="); debug_serial_print(chunk);
    debug_serial_print(" fr_sv1="); debug_serial_print(fr_sv[1]);
    debug_serial_print(" fr_sv2="); debug_serial_print(fr_sv[2]);
    debug_serial_print(" fr_sv3="); debug_serial_print(fr_sv[3]);   
    debug_serial_print(" fr_sv4="); debug_serial_print(fr_sv[4]); 
    debug_serial_print(" fr_payload="); debug_serial_print(fr_payload);  debug_serial_print(" "); 
    DisplayPayload(fr_payload);
    debug_serial_println();             
  #endif

  sv_count += 4; 
}
//=================================================================================================  
void Pack_VFR_Hud_50F2(uint16_t id) {
  uint32_t fr_payload = 0;
  
  fr_air_spd = ap_hud_air_spd * 10;      // from #74  m/s to dm/s
  fr_throt = ap_hud_throt;               // 0 - 100%
  fr_bar_alt = ap_hud_bar_alt * 10;      // m to dm

  #if defined Frs_Debug_All || defined Frs_Debug_Hud
    ShowPeriod(0);  
    debug_serial_print("FrSky in Hud 0x50F2: ");   
    debug_serial_print(" fr_air_spd="); debug_serial_print(fr_air_spd);
    debug_serial_print(" fr_throt="); debug_serial_print(fr_throt);
    debug_serial_print(" fr_bar_alt="); debug_serial_print(fr_bar_alt);
    debug_serial_print(" fr_payload="); debug_serial_print(fr_payload);  debug_serial_print(" "); 
    DisplayPayload(fr_payload);
    debug_serial_println();             
  #endif
  
  fr_air_spd = prep_number(roundf(fr_air_spd), 2, 1);  
  bit32Pack(fr_air_spd, 0, 8, &fr_payload);    

  bit32Pack(fr_throt, 8, 7, &fr_payload);

  fr_bar_alt =  prep_number(roundf(fr_bar_alt), 3, 2);
  bit32Pack(fr_bar_alt, 15, 12, &fr_payload);
  if (fr_bar_alt < 0)
    bit32Pack(1, 27, 1, &fr_payload);  
  else
   bit32Pack(0, 27, 1, &fr_payload); 
    
  sr.id = id;   
  sr.subid = 1;
  sr.payload = fr_payload;
  PushToEmptyRow(sr); 
        
}
//=================================================================================================          
void Pack_Rssi_F101(uint16_t id) {          // data id 0xF101 RSSI tell LUA script in Taranis we are connected

  uint32_t fr_payload = 0;
  bit32Pack((uint32_t)ap_rssi ,0, 32, &fr_payload);

  #if defined Frs_Debug_All || defined Debug_Rssi
    ShowPeriod(0);    
    debug_serial_print("FrSky in Rssi 0x5F101: ");   
    debug_serial_print(" fr_rssi="); debug_serial_print(fr_rssi);
    debug_serial_print(" fr_payload="); debug_serial_print(fr_payload);  debug_serial_print(" "); 
    DisplayPayload(fr_payload);
    debug_serial_println();             
  #endif

  sr.id = id;
  sr.subid = 1;
  sr.payload = fr_payload;
  PushToEmptyRow(sr); 
}
//=================================================================================================  
int8_t PWM_To_63(uint16_t PWM) {       // PWM 1000 to 2000   ->    nominal -63 to 63
int8_t myint;
  myint = round((PWM - 1500) * 0.126); 
  myint = myint < -63 ? -63 : myint;            
  myint = myint > 63 ? 63 : myint;  
  return myint; 
}

//=================================================================================================  
uint32_t Abs(int32_t num) {
  if (num<0) 
    return (num ^ 0xffffffff) + 1;
  else
    return num;  
}
//=================================================================================================  
float Distance(Loc2D loc1, Loc2D loc2) {
float a, c, d, dLat, dLon;  

  loc1.lat=loc1.lat/180*PI;  // degrees to radians
  loc1.lon=loc1.lon/180*PI;
  loc2.lat=loc2.lat/180*PI;
  loc2.lon=loc2.lon/180*PI;
    
  dLat = (loc1.lat-loc2.lat);
  dLon = (loc1.lon-loc2.lon);
  a = sin(dLat/2) * sin(dLat/2) + sin(dLon/2) * sin(dLon/2) * cos(loc2.lat) * cos(loc1.lat); 
  c = 2* asin(sqrt(a));  
  d = 6371000 * c;    
  return d;
}
//=================================================================================================  
float Azimuth(Loc2D loc1, Loc2D loc2) {
// Calculate azimuth bearing from loc1 to loc2
float a, az; 

  loc1.lat=loc1.lat/180*PI;  // degrees to radians
  loc1.lon=loc1.lon/180*PI;
  loc2.lat=loc2.lat/180*PI;
  loc2.lon=loc2.lon/180*PI;

  a = sin(dLat/2) * sin(dLat/2) + sin(dLon/2) * sin(dLon/2) * cos(loc2.lat) * cos(loc1.lat); 
  
  az=a*180/PI;  // radians to degrees
  if (az<0) az=360+az;
  return az;
}
//=================================================================================================  
//Add two bearing in degrees and correct for 360 boundary
int16_t Add360(int16_t arg1, int16_t arg2) {  
  int16_t ret = arg1 + arg2;
  if (ret < 0) ret += 360;
  if (ret > 359) ret -= 360;
  return ret; 
}
//=================================================================================================  
// Correct for 360 boundary - yaapu
float wrap_360(int16_t angle)
{
    const float ang_360 = 360.f;
    float res = fmodf(static_cast<float>(angle), ang_360);
    if (res < 0) {
        res += ang_360;
    }
    return res;
}
//=================================================================================================  
// From Arducopter 3.5.5 code
uint16_t prep_number(int32_t number, uint8_t digits, uint8_t power)
{
    uint16_t res = 0;
    uint32_t abs_number = abs(number);

   if ((digits == 1) && (power == 1)) { // number encoded on 5 bits: 4 bits for digits + 1 for 10^power
        if (abs_number < 10) {
            res = abs_number<<1;
        } else if (abs_number < 150) {
            res = ((uint8_t)roundf(abs_number * 0.1f)<<1)|0x1;
        } else { // transmit max possible value (0x0F x 10^1 = 150)
            res = 0x1F;
        }
        if (number < 0) { // if number is negative, add sign bit in front
            res |= 0x1<<5;
        }
    } else if ((digits == 2) && (power == 1)) { // number encoded on 8 bits: 7 bits for digits + 1 for 10^power
        if (abs_number < 100) {
            res = abs_number<<1;
        } else if (abs_number < 1270) {
            res = ((uint8_t)roundf(abs_number * 0.1f)<<1)|0x1;
        } else { // transmit max possible value (0x7F x 10^1 = 1270)
            res = 0xFF;
        }
        if (number < 0) { // if number is negative, add sign bit in front
            res |= 0x1<<8;
        }
    } else if ((digits == 2) && (power == 2)) { // number encoded on 9 bits: 7 bits for digits + 2 for 10^power
        if (abs_number < 100) {
            res = abs_number<<2;
         //   debug_serial_print("abs_number<100  ="); debug_serial_print(abs_number); debug_serial_print(" res="); debug_serial_print(res);
        } else if (abs_number < 1000) {
            res = ((uint8_t)roundf(abs_number * 0.1f)<<2)|0x1;
         //   debug_serial_print("abs_number<1000  ="); debug_serial_print(abs_number); debug_serial_print(" res="); debug_serial_print(res);
        } else if (abs_number < 10000) {
            res = ((uint8_t)roundf(abs_number * 0.01f)<<2)|0x2;
          //  debug_serial_print("abs_number<10000  ="); debug_serial_print(abs_number); debug_serial_print(" res="); debug_serial_print(res);
        } else if (abs_number < 127000) {
            res = ((uint8_t)roundf(abs_number * 0.001f)<<2)|0x3;
        } else { // transmit max possible value (0x7F x 10^3 = 127000)
            res = 0x1FF;
        }
        if (number < 0) { // if number is negative, add sign bit in front
            res |= 0x1<<9;
        }
    } else if ((digits == 3) && (power == 1)) { // number encoded on 11 bits: 10 bits for digits + 1 for 10^power
        if (abs_number < 1000) {
            res = abs_number<<1;
        } else if (abs_number < 10240) {
            res = ((uint16_t)roundf(abs_number * 0.1f)<<1)|0x1;
        } else { // transmit max possible value (0x3FF x 10^1 = 10240)
            res = 0x7FF;
        }
        if (number < 0) { // if number is negative, add sign bit in front
            res |= 0x1<<11;
        }
    } else if ((digits == 3) && (power == 2)) { // number encoded on 12 bits: 10 bits for digits + 2 for 10^power
        if (abs_number < 1000) {
            res = abs_number<<2;
        } else if (abs_number < 10000) {
            res = ((uint16_t)roundf(abs_number * 0.1f)<<2)|0x1;
        } else if (abs_number < 100000) {
            res = ((uint16_t)roundf(abs_number * 0.01f)<<2)|0x2;
        } else if (abs_number < 1024000) {
            res = ((uint16_t)roundf(abs_number * 0.001f)<<2)|0x3;
        } else { // transmit max possible value (0x3FF x 10^3 = 127000)
            res = 0xFFF;
        }
        if (number < 0) { // if number is negative, add sign bit in front
            res |= 0x1<<12;
        }
    }
    return res;
}  
//=================================================================================================  
//================================================================================================= 
//
//                                       U T I L I T I E S
//
//================================================================================================= 
//=================================================================================================

struct Battery {
  float    mAh;
  float    tot_mAh;
  float    avg_dA;
  float    avg_mV;
  uint32_t prv_millis;
  uint32_t tot_volts;      // sum of all samples
  uint32_t tot_mW;
  uint32_t samples;
  bool ft;
  };
  
struct Battery bat1     = {
  0, 0, 0, 0, 0, 0, 0, true};   

struct Battery bat2     = {
  0, 0, 0, 0, 0, 0, 0, true};   

//=================================================================================================  
void DisplayByte(byte b) {
  if (b<=0xf) ;debug_serial_print("0");
  debug_serial_print(String(b,HEX));
  debug_serial_print(" ");
}
//=================================================================================================  
float RadToDeg (float _Rad) {
  return _Rad * 180 / PI;  
}
//=================================================================================================  
float DegToRad (float _Deg) {
  return _Deg * PI / 180;  
}
//=================================================================================================  
#if defined Mav_Debug_All || defined Mav_Debug_StatusText
String MavSeverity(uint8_t sev) {
 switch(sev) {
    
    case 0:
      return "EMERGENCY";     // System is unusable. This is a "panic" condition. 
      break;
    case 1:
      return "ALERT";         // Action should be taken immediately. Indicates error in non-critical systems.
      break;
    case 2:
      return "CRITICAL";      // Action must be taken immediately. Indicates failure in a primary system.
      break; 
    case 3:
      return "ERROR";         //  Indicates an error in secondary/redundant systems.
      break; 
    case 4:
      return "WARNING";       //  Indicates about a possible future error if this is not resolved within a given timeframe. Example would be a low battery warning.
      break; 
    case 5:
      return "NOTICE";        //  An unusual event has occured, though not an error condition. This should be investigated for the root cause.
      break;
    case 6:
      return "INFO";          //  Normal operational messages. Useful for logging. No action is required for these messages.
      break; 
    case 7:
      return "DEBUG";         // Useful non-operational messages that can assist in debugging. These should not occur during normal operation.
      break; 
    default:
      return "UNKNOWN";                                          
   }
}
//=================================================================================================  
String PX4FlightModeName(uint8_t main, uint8_t sub) {
 switch(main) {
    
    case 1:
      return "MANUAL"; 
      break;
    case 2:
      return "ALTITUDE";        
      break;
    case 3:
      return "POSCTL";      
      break; 
    case 4:
 
      switch(sub) {
        case 1:
          return "AUTO READY"; 
          break;    
        case 2:
          return "AUTO TAKEOFF"; 
          break; 
        case 3:
          return "AUTO LOITER"; 
          break;    
        case 4:
          return "AUTO MISSION"; 
          break; 
        case 5:
          return "AUTO RTL"; 
          break;    
        case 6:
          return "AUTO LAND"; 
          break; 
        case 7:
          return "AUTO RTGS"; 
          break;    
        case 8:
          return "AUTO FOLLOW ME"; 
          break; 
        case 9:
          return "AUTO PRECLAND"; 
          break; 
        default:
          return "AUTO UNKNOWN";   
          break;
      } 
      
    case 5:
      return "ACRO";
    case 6:
      return "OFFBOARD";        
      break;
    case 7:
      return "STABILIZED";
      break;        
    case 8:
      return "RATTITUDE";        
      break; 
    case 9:
      return "SIMPLE";  
    default:
      return "UNKNOWN";
      break;                                          
   }
}
#endif
//=================================================================================================  
uint8_t PX4FlightModeNum(uint8_t main, uint8_t sub) {
  if (main < 10) {
    if (main < 4) {
      return main - 1;
    } else if (main > 4) {
      return main - 2;
    } else {
      if (sub < 10) {
        return sub + 11;
      } else return 31;
    }
  } else return 11;

 /*switch(main) {
    
    case 1:
      return 0;  // MANUAL 
    case 2:
      return 1;  // ALTITUDE       
    case 3:
      return 2;  // POSCTL      
    case 4:
 
      switch(sub) {
        case 1:
          return 12;  // AUTO READY
        case 2:
          return 13;  // AUTO TAKEOFF 
        case 3:
          return 14;  // AUTO LOITER  
        case 4:
          return 15;  // AUTO MISSION 
        case 5:
          return 16;  // AUTO RTL 
        case 6:
          return 17;  // AUTO LAND 
        case 7:
          return 18;  //  AUTO RTGS 
        case 8:
          return 19;  // AUTO FOLLOW ME 
        case 9:
          return 20;  //  AUTO PRECLAND 
        default:
          return 31;  //  AUTO UNKNOWN   
      } 
      
    case 5:
      return 3;  //  ACRO
    case 6:
      return 4;  //  OFFBOARD        
    case 7:
      return 5;  //  STABILIZED
    case 8:
      return 6;  //  RATTITUDE        
    case 9:
      return 7;  //  SIMPLE 
    default:
      return 11;  //  UNKNOWN                                        
   }*/
}

//=================================================================================================  
#if defined Frs_Debug_All
void ShowPeriod(bool LF) {
  static uint32_t now_millis = 0;
  static uint32_t prev_millis = 0;
  debug_serial_print("Period ms=");
  now_millis=millis();
  debug_serial_print(now_millis-prev_millis);
  if (LF) {
    debug_serial_print("\t\n");
  } else {
   debug_serial_print("\t");
  }
    
  prev_millis=now_millis;
}
#endif

uint32_t Get_Volt_Average1(uint16_t mV)  {

  if (bat1.avg_mV < 1) bat1.avg_mV = mV;  // Initialise first time

 // bat1.avg_mV = (bat1.avg_mV * 0.9) + (mV * 0.1);  // moving average
  bat1.avg_mV = (bat1.avg_mV * 0.6666) + (mV * 0.3333);  // moving average
  Accum_Volts1(mV);  
  return bat1.avg_mV;
}
//=================================================================================================  
uint32_t Get_Current_Average1(uint16_t dA)  {   // in 10*milliamperes (1 = 10 milliampere)
  
  Accum_mAh1(dA);  
  
  if (bat1.avg_dA < 1){
    bat1.avg_dA = dA;  // Initialise first time
  }

  bat1.avg_dA = (bat1.avg_dA * 0.6666) + (dA * 0.333);  // moving average

  return bat1.avg_dA;
  }

void Accum_Volts1(uint32_t mVlt) {    //  mV   milli-Volts
  bat1.tot_volts += (mVlt / 1000);    // Volts
  bat1.samples++;
}

void Accum_mAh1(uint32_t dAs) {        //  dA    10 = 1A
  if (bat1.ft) {
    bat1.prv_millis = millis() -1;   // prevent divide zero
    bat1.ft = false;
  }
  uint32_t period = millis() - bat1.prv_millis;
  bat1.prv_millis = millis();
    
  double hrs = (float)(period / 3600000.0f);  // ms to hours

  bat1.mAh = dAs * hrs;     //  Tiny dAh consumed this tiny period di/dt
 // bat1.mAh *= 100;        //  dA to mA  
  bat1.mAh *= 10;           //  dA to mA ?
  bat1.mAh *= 1.0625;       // Emirical adjustment Markus Greinwald 2019/05/21
  bat1.tot_mAh += bat1.mAh;   //   Add them all in
}

float Total_mAh1() {
  return bat1.tot_mAh;
}

float Total_mWh1() {                                     // Total energy consumed bat1
  return bat1.tot_mAh * (bat1.tot_volts / bat1.samples);
}
//=================================================================================================  
uint32_t Get_Volt_Average2(uint16_t mV)  {
  
  if (bat2.avg_mV == 0) bat2.avg_mV = mV;  // Initialise first time

  bat2.avg_mV = (bat2.avg_mV * 0.666) + (mV * 0.333);  // moving average
  Accum_Volts2(mV);  
  return bat2.avg_mV;
}
  
uint32_t Get_Current_Average2(uint16_t dA)  {

  if (bat2.avg_dA == 0) bat2.avg_dA = dA;  // Initialise first time

  bat2.avg_dA = (bat2.avg_dA * 0.666) + (dA * 0.333);  // moving average

  Accum_mAh2(dA);  
  return bat2.avg_dA;
  }

void Accum_Volts2(uint32_t mVlt) {    //  mV   milli-Volts
  bat2.tot_volts += (mVlt / 1000);    // Volts
  bat2.samples++;
}

void Accum_mAh2(uint32_t dAs) {        //  dA    10 = 1A
  if (bat2.ft) {
    bat2.prv_millis = millis() -1;   // prevent divide zero
    bat2.ft = false;
  }
  uint32_t period = millis() - bat2.prv_millis;
  bat2.prv_millis = millis();
    
 double hrs = (float)(period / 3600000.0f);  // ms to hours

  bat2.mAh = dAs * hrs;   //  Tiny dAh consumed this tiny period di/dt
 // bat2.mAh *= 100;        //  dA to mA  
  bat2.mAh *= 10;        //  dA to mA ?
  bat2.mAh *= 1.0625;       // Emirical adjustment Markus Greinwald 2019/05/21 
  bat2.tot_mAh += bat2.mAh;   //   Add them all in
}

float Total_mAh2() {
  return bat2.tot_mAh;
}

float Total_mWh2() {                                     // Total energy consumed bat1
  return bat2.tot_mAh * (bat2.tot_volts / bat2.samples);
}
//=================================================================================================  
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
#include <Arduino.h>
#include "SmartSerial.h"
#include "rfd900x.h"
#include "led.h"

HardwareSerial Serial9x(1); //  attached 900x device is on 'Serial' and instantiated as a Serial9x object. 
File f; // global handle use for the Xmodem upload
XModem xmodem(&Serial, ModeXModem);
MySerial *SmartSerial = new MySerial(&Serial9x); 
static uint8_t r9x_sensitivity = 94; // Defaults to -105dBm seen on 64kpbs datarates
const char * rfd_sik_sig = "RFD SiK";

void r900x_initiate_serials(void) {
    delay(100);
    Serial.begin(115200); 
    debug_init();

    SmartSerial->begin();
    debug_serial_println(F("doing setup()"));
}

void flush_rx_serial() {
    while (Serial.available() ) { 
        #ifdef DEBUG_DISABLE
        Serial.read();
        #else
        char t = Serial.read();  
        debug_serial_print(t); 
        #endif
    } // flush read buffer upto this 
}

//-------------------------------------------
int r900x_booloader_mode_sync() { 

debug_serial_println(F("r900x_booloader_mode_sync()"));
      Serial.write("U"); // autobauder
      Serial.flush(); 

      int ok = SmartSerial->expect_multi3("ChipID:", "UPLOAD", "XXXXXXXXXXX",500); // we're really looking for a ChipID, but UPLOAD will do too, and XXX is an unused parameter
      if ( ok > 0 ) { 
          debug_serial_println(F("\t\tGOT ChipID/UPLOAD Response from radio.\n"));
          flush_rx_serial();
          return ok;
      } else { 
          debug_serial_println(F("\t\tFAILED-TO-GET ChipID/UPLOAD Response from radio.\n"));
          return -1;
      }       
}

//-------------------------------------------

// this improved version is used consistently across the code now, as it has a collection of optimisations
// that include a quick ATI check first ( with very short timeout ) before doing a long +++ timeout if needed.
// and it toggles LEDS, and it also terminates the +++ command afterwards, with \r\n as needed.
bool enter_command_mode() { 

    // to make it potentially faster for the majority of cases where the radio is *still* in 
    // command-mode, we could quickly try an 'ATI' and if we get an echo back along with the 
    // version info, were already in command mode and don't have to wait a whone second to 
    // find out.
    //ATI
    //RFD SiK 3.48 on RFD900X2
    Serial.write("\r\nATI\r\n");
    Serial.flush(); // output buffer flush
    delay(100);

    int already_commmand_mode_test = SmartSerial->expect_multi3("ATI", rfd_sik_sig, "RFD900X2", 250); // look for any of these

    flush_rx_serial();

    if (  already_commmand_mode_test <= 0 ) {  // no match on the above 3 strings means ...

        // put it into command mode first....
        Serial.write("\r\n");
        Serial.flush();
        delay(1050); 
        Serial.write("+++");
        Serial.flush(); // output buffer flush

        // toggle LED to be more interesting.
        toggle_led_state();

        int ok2 = SmartSerial->expect_multi3("OK","+++","XXXXX",1150); // look for +++ takes 50ms, OK might take 1000 ?
        if ( ok2 == 1 ) { 
            debug_serial_println(F("GOT OK Response from radio."));
            flush_rx_serial();
            return true;
        }
        else if ( ok2 == 2 ) { 
            // if modem echo's back the +++ we sent it, we are already in command mode.
            debug_serial_println(F("radio is already_in_cmd_mode2, continuing... sending RN."));
            delay(100);
            Serial.write("\r\n"); // terminate the +++ command.
            Serial.flush(); // output buffer flush
            delay(100);
            //flush_rx_serial();
            return true;
        } else { 
            debug_serial_println(F("FALED-TO-GET 'OK' or '+++' Response from radio."));
            debug_serial_println(F("RETURN:no cmd-mode modem coms")); 
            return false;  
        } 

    }

    // if already_commmand_mode_test > 0 
    return true;
    

}

//-------------------------------------------
// warning execution time on this can be as much as ~4 seconds if the device is repeatly refusing to enter command mode with +++
bool enter_command_mode_with_retries() { 

    bool result = enter_command_mode(); // try 1

    debug_serial_println(result?"true":"false");

    if ( result ) return true;

    int chances = 0;

    while (( ! result) && (chances < 3 )) {  // chances 0,1,2

        result = enter_command_mode(); // try 2,3,4

        debug_serial_println(result?"re-true":"re-false");

        if ( result ) return true;

        chances++;
    }
    return false;
} 

// prerequisite, u should have called some form of enter_command_mode() before this one:
int r900x_readsingle_param_impl( String prefix, String ParamID ) { 
    debug_serial_print(F("r900x_readsingle_param_impl "));

    //  read individial param first  ( eg ATS4? ) to see if it even needs changing
    String cmd = prefix+ParamID+"?\r\n"; // first \r\n is to end the +++ command we did above, if any.


    // shortcurcuit for &E etc
    if ( ParamID == F("&E") ) return -4; // not readable, for now. 
    if ( ParamID == F("&R") ) return -4; // not readable, for now. 
    if ( ParamID == F("&F") ) return -4; // not readable, for now. 

    debug_serial_print(cmd); // debug only

    // here we neter a "retries" loop for approx 100ms each iteration and maxloops 5
    // because it's proven in testing that the RTS3?   and similar style commands don't 
    // work first-go , perhaps 2nd go? 
    // also, this doesn't work if the remote radio isn't there and it's a RT command
    String data = "";
    String data3 = "";
    int max = 0;
    // TODO does not handle RT&E? params. as it returns a long string with 32 hex chars.
    while (( data.length() < 10 ) && (max < 5 ) && ( data3.length() <= 2 ) )  { 
        Serial.write(cmd.c_str());
        Serial.flush(); // output buffer flush
        data = SmartSerial->expect_s(cmd,55);  // this should capture the echo of the RTS3?
        data3 = SmartSerial->expect_s("\r\n",500);  // this should capture the line after and its contents
        // go to first character which is an number 0-9
        int i=0;
        for(i=0; i< data3.length();i++){if(isDigit(data3.charAt(i)))break;}
        data3 = data3.substring(i);
        //debug_serial_println(F("^^^^^^^^^^^^^^^^^^^"));
        //debug_serial_println(max);
        max++;
    }

    if (data3.length() <= 2 ) { 
        debug_serial_println(F("RETURN:existing val cant be read")); 
        return -4;  
    } 
    // if we didn't get a number and \r and \n, then give up.    

    // TODO, what if we hit retry limit ? 
    //if ( max == 5 ) {  ... } 

    //here  - decide if param valus is already set right, and if so, return OK status as it's verified! 
    // data3 contains the stored value for the ParamID in the prefix-type radio., with newlines and as a string.
    int value = data3.toInt(); 

    debug_serial_print(data);
    debug_serial_println(String(value));
   
    return value; 
}

//-------------------------------------------
// no prerequisite version of the above function. with more retries. 
int r900x_readsingle_param(String prefix, String ParamID) { 

    debug_serial_print(F("r900x_readsingle_param "));

    if ( ! enter_command_mode_with_retries() ) { return -1 ; } 

    //debug_serial_println(F("Z-------flush---------------"));
    flush_rx_serial();

    // might return negative number on error.... so add one ghetto retry here as it already has some retrues...
    int retval =  r900x_readsingle_param_impl( prefix, ParamID );

    if ( retval >= 0 ) return retval;

    if ( retval < 0 ) { 
    
        delay(500);

        // erp, should not be needed, but aparently can be...
        String exitcmd= "ATO\r\n";
        debug_serial_print(exitcmd); // debug only.
        Serial.write(exitcmd.c_str());
        Serial.flush(); // output buffer flush

        debug_serial_print(F("r900x_readsingle_param-RETRY "));

        if ( ! enter_command_mode_with_retries() ) { return retval ; } 

        //debug_serial_println(F("Q-------flush---------------"));
        //flush_rx_serial();

        // last try, might return negative number on error, just return it.. 
        return r900x_readsingle_param_impl( prefix, ParamID );
    }
    return retval; // never reached but removes compiler warning
}

//-------------------------------------------
// returns negative numbers on error, and positive number on success
//        String prefix = "AT";  or RT
//        String ParamID = line.substring(0,colon_offset); // S4  or E?
//        String ParamNAME = line.substring(colon_offset+1,equals_offset); // TXPOWER
//        String ParamVAL = line.substring(equals_offset+1,eol_offset);  // 30

int r900x_savesingle_param_and_verify_more(String prefix, String ParamID, String ParamVAL, bool save_and_reboot) { 

    debug_serial_print(F("r900x_savesingle_param_and_verify "));

    // sometimes we skip parts of these see below
    bool actually_write = true;


    if ( ! enter_command_mode_with_retries() ) { return -1 ; } 

    //debug_serial_println(F("X-------flush---------------"));
    flush_rx_serial();

    // might return negative number on error.... ( such as &E being empty, or &R not being standard )
    int value = r900x_readsingle_param_impl( prefix, ParamID );
    
    // success, it's already set at that target val  
    // this  honors the save_and_reboot parameter by jumping to the save-and-reboot section, which it should.  
    if (value == ParamVAL.toInt() ) { 
        if ( save_and_reboot ) {
            debug_serial_println(F("val already set, jumping to save-and-reboot anyway"));
            actually_write = false;   //continues below
        } else {
            debug_serial_println(F("RETURN:val already set, no save/reboot requested"));
            return 1;   // shortcurcuit and return immediately
        }
    } 

    // unreadable error... for now we try to write it anyway.
    //if ( value < 0 ) {     }

    if (  actually_write ) { 
        int param_write_counter = 0;
        while (param_write_counter < 5) 
        {
            // set param to new vaue
            String ParamCMD= prefix+ParamID+"="+ParamVAL+"\r\n";

                // overwride for special case/s ( very rare ) 
                if (( prefix == F("RT") ) && ( ParamID == F("&R") )) ParamCMD = prefix+ParamID+"\r\n";
                if ( ParamID == F("&F") ) ParamCMD = prefix+ParamID+"\r\n";

            debug_serial_print(ParamCMD); // debug only.
            Serial.write(ParamCMD.c_str());
            Serial.flush(); // output buffer flush
            bool ok = SmartSerial->expect("OK",1000);  // needs to be bigger than 200.
            //debug_serial_println(F("7#############################################################"));
            if ( ok ) { 
                debug_serial_println(F("GOT OK from radio.\n"));
                flush_rx_serial();
                break;
            } else { 

                // HACK. we don't get an OK reply from RT&E=XXXXXXXXXXXXX, just an echo 
                //of the command and a blat of suddenly unreadable bytes and the 250ms timeout.
                //if (( ParamID == F("&E") ) && ( param_write_counter > 8 ) ) { 
                //   return 3; // positve number = success.
                //}

                debug_serial_println(F("FALED-TO-GET OK Response from radio. - retrying:\n"));
                param_write_counter++;                
            }
        } 

        if (param_write_counter == 5) {
            // if 5 retries exceeded, return error.
            debug_serial_println(F("RETURN:no response to param set request - 5 retries exceeded")); 
            return -2; // neg number/s mean error
        }

        if ( ! save_and_reboot ) {
            debug_serial_println(F("Skipping save and reboot RETURN:PERFECTO"));
            return 3;
        } 
    }

    if ( save_and_reboot )  { 
        int param_save_counter = 0;
        while (param_save_counter < 5) {

            // save params AND take out of command mode via a quick reboot
            String savecmd = prefix+"&W\r\n"; // "AT&W\r\n
            debug_serial_print (savecmd); // debug only.
            Serial.write(savecmd.c_str());
            Serial.flush(); // output buffer flush

            bool ok = SmartSerial->expect("OK",1000); 
            if ( ok ) { 
                debug_serial_println(F(" GOT OK Response from radio.\n"));
                //flush_rx_serial();

                // save params AND take out of command mode via a quick reboot
                String rebootcmd = prefix+"Z\r\n"; // "ATZ\r\n"
                debug_serial_print(rebootcmd); // debug only.
                Serial.write(rebootcmd.c_str());
                Serial.flush(); // output buffer flush

                delay(150);
                // TODO ? ok = SmartSerial->expect("RTZ",200);
                flush_rx_serial(); // - ie 'RTZ' response
                break;

            } else { 
                debug_serial_println(F("FALED-TO-GET OK Response from radio. - retrying:\n"));
                delay(100); // might help by delaying the retry a bit.
                param_save_counter++;
            } 

            if (param_save_counter == 5) {
                // if 5 retries exceeded, return error.
                debug_serial_println(F("RETURN:no response to param save request - 5 retries exceeded")); 
                return -3;
            }
        }
    }

    debug_serial_println(F("RETURN:PERFECT"));
    return 2;
}
//-------------------------------------------
int r900x_savesingle_param_and_verify(String prefix, String ParamID, String ParamVAL) { 
    return r900x_savesingle_param_and_verify_more( prefix,  ParamID,  ParamVAL, true);
}

//-------------------------------------------

int r900x_readcachedparam(String filename, String ParameterName) {
    // Reads a single parameter from the
    // cached parameters file. It returns a
    // negative number in case of failure.

    // opens the file
    File f = SPIFFS.open(filename, "r");
    f.setTimeout(200);

    if (f) {
        int done = 0;

        // Let's assume there will not be more than 100 lines,
        // so that we can exit the loop if stuck.
        while (done < 100){
            String line = f.readStringUntil('\n'); // read a line, wait max 100ms.

            int colon_offset = line.indexOf(":"); // it should have a colon, and an = sign
            int equals_offset = line.indexOf("="); // it should have a colon, and an = sign
            int eol_offset = line.indexOf("\r"); // line ends with \r\n. this finds the first of these

            // if all of them is -1, it's the end of the file.
            if (( colon_offset == -1 ) && ( equals_offset == -1 ) && ( eol_offset == -1 ) )
                return -1;

            //  if any of these is -1, it failed, just skip that line
            if (( colon_offset == -1 ) || ( equals_offset == -1 ) || ( eol_offset == -1 ) )
                continue;

            String ParamID = line.substring(0,colon_offset);
            String ParamNAME = line.substring(colon_offset+1,equals_offset);
            String ParamVAL = line.substring(equals_offset+1,eol_offset); 

            // if its an ATS2 or RTS0 command, skipp it, as we don't allows writes to S0
            if ( ParamNAME == ParameterName )
                return ParamVAL.toInt();
        }

        // No parameter could be found with a matching name
        return -2;
    }

    // SPIFFS could not open the filename provided
    return -3;
}

//-------------------------------------------

bool r900x_saveparams(String filename) { 
debug_serial_println(F("r900x_saveparams()"));
// iterate over the params found in r900x_params.txt and save them to the modem as best as we can.


    if ( ! enter_command_mode_with_retries() ) { return -1; } 
 
    String localfilename = RFD_LOC_PAR;
    String remotefilename = RFD_REM_PAR;

    File f = SPIFFS.open(filename, "r");    

    String prefix = "AT";
    if ( filename == remotefilename ) { prefix = "RT"; } 

    f.setTimeout(200); // don't wait long as it's a file object, not a serial port.

    if ( ! f ) {  // did we open this file ok, ie does it exist? 
        debug_serial_print (F("no txt file exists, can't update modem settings, skipping:"));
        debug_serial_println(filename);
        return false;
    }

    int done = 0; 

    while ( done < 30 ) { 

        toggle_led_state();

        String line = f.readStringUntil('\n'); // read a line, wait max 100ms.

        if (line.substring(0,3) == F("ATI") ) { continue; } // ignore this first line.
        if (line.substring(0,3) == F("RTI") ) { continue; } // ignore this first line.

        //if (line == "\r\n" ) { continue; } // ignore blank line/s
        
        int colon_offset = line.indexOf(":"); // it should have a colon, and an = sign
        int equals_offset = line.indexOf("="); // it should have a colon, and an = sign
        int eol_offset = line.indexOf("\r"); // line ends with \r\n. this finds the first of these

        // if all of them is -1, it's the end of the file.
        if (( colon_offset == -1 ) && ( equals_offset == -1 ) && ( eol_offset == -1 ) ) {    
            break;
        } 

        //  if any of these is -1, it failed, just skip that line
        if (( colon_offset == -1 ) || ( equals_offset == -1 ) || ( eol_offset == -1 ) ) {    
            debug_serial_println(F("skipping line as it didn't parse well."));
            continue;
        } 


        String ParamID = line.substring(0,colon_offset);
        String ParamNAME = line.substring(colon_offset+1,equals_offset);
        String ParamVAL = line.substring(equals_offset+1,eol_offset); 

        // if its an ATS2 or RTS0 command, skipp it, as we don't allows writes to S0
        if ( ParamID == F(PARAM_FORMAT_STR) ) { 
            debug_serial_println(F("skipping S0, as we don't write it."));
            continue; 
        }

        // basic retry counter for 
        int param_write_counter2 = 0;
        param_write_retries2:
        
        String ParamCMD= prefix+ParamID+"="+ParamVAL+"\r\n";

        // we implement retries on ths for if we didn't get a response to the command in-time..,
        // and becasue AT and RT commands are notoriously non-guaranteed.

        debug_serial_print(ParamCMD); // debug only.
        Serial.write(ParamCMD.c_str());
        Serial.flush(); // output buffer flush
        bool ok = SmartSerial->expect("OK",200); // typically we get a remote response in under 150ms, local response under 30
        if ( ok ) { 
            debug_serial_println(F(" GOT OK from radio."));
            flush_rx_serial();
        } else { 

            debug_serial_println(F("FALED-TO-GET OK Response from radio. - retrying:"));
            param_write_counter2++;
            if ( param_write_counter2 < 5 ) { 
                goto param_write_retries2;
            }
            // if ~5 retries exceeded, return error.
            debug_serial_println(F("RETURN:no response to param set request - 5 retries exceeded")); 
            return -2; // neg number/s mean error

        } 
        
        done++;
    }   

   // save params AND maybe take out of command mode via a quick reboot
    String savecmd = prefix+"&W\r\n"; // "AT&W\r\n
    debug_serial_print(savecmd); // debug only.
    Serial.write(savecmd.c_str());
    Serial.flush(); // output buffer flush

    set_led_state(false);

    bool ok = SmartSerial->expect("OK",2000); 
    if ( ok ) { 
        debug_serial_println(F("GOT OK Response from radio.\n"));
        //flush_rx_serial();

          // take out of command mode via a quick reboot
            String rebootcmd = prefix+"Z\r\n"; // "ATZ\r\n"
            debug_serial_println(rebootcmd); // debug only.
            Serial.write(rebootcmd.c_str());
            Serial.flush(); // output buffer flush

            set_led_state(0);

            delay(1000); // hack to allow the radio to come back and sync after a ATZ or RTZ
 
            //ok = SmartSerial->expect("OK",2000);  we don't expect a response from ATZ or RTZ
    } else { 
        //trying = false; HACK
        debug_serial_println(F("FALED-TO-GET OK Response from radio, no biggie, reboot expected.\n"));
        //return false;
    } 
    return true;  
} 



bool got_hex = false;
bool r900x_command_mode_sync() { 
        debug_serial_println(F("r900x_command_mode_sync()\n"));
        debug_serial_println(F("Trying command-mode sync....\n"));
    
        // try to put radio into command mode with +++, but if no response, carry on anyway, as it's probably in bootloader mode already.
        enter_command_mode() ; // don't giveup if this returns false

        // we try the ATI style commands up to 2 times to get a 'sync' if needed. ( was 5, but 2 is faster )
        for (int i=0; i < 2; i++) {

            toggle_led_state();

            debug_serial_print(F("\tATI Attempt Number: ")); debug_serial_println(i);
            debug_serial_print("\tSending ATI to radio. ");
            //
            Serial.write("ATI\r\n");
            Serial.flush(); // output buffer flush

            int ok2 = SmartSerial->expect_multi3("SiK","\xC1\xE4\xE3\xF8","\xC1\xE4\xE7\xF8",2000);  // we really want to see 'Sik' here, but if we see the hex string/s we cna short-curcuit 

            if ( ok2 == 1 ) { 
                debug_serial_println(F("\tGOT SiK Response from radio."));
            } 
            if ( ok2 == 2 ) { 
                debug_serial_println(F("\tGOT bootloader HEX (C1 E4 E3 F8 ) Response from radio."));
                got_hex = true;
                return false;
            } 
            if ( ok2 == 3 ) { 
                debug_serial_println(F("\tGOT bootloader HEX (C1 E4 E7 F8 ) Response from radio."));
                got_hex = true;
                return false;
            } 

            if ( ok2 == -1 ) { 
                debug_serial_println(F("\tFAILED-TO-GET SiK Response from radio."));
                continue;
            } 

            debug_serial_print(F("\t\tSending AT&UPDATE to radio..."));
            //
            Serial.write("\r\n");
            delay(200); 
            Serial.flush();
            Serial.write("AT&UPDATE\r"); // must be \r only, do NOT include \n here
            delay(700); 
            Serial.flush();

            debug_serial_println(F("Sent update command"));

            return true;
        }
  return false;
}

//-------------------------------------------
// returns -1 on major error, positive number of params fetched on 'success'
int r900x_getparams(String filename, bool factory_reset_first) { 

    debug_serial_print(F("r900x_getparams("));
    debug_serial_print(filename);
    debug_serial_print(F(")- START "));
    
    debug_serial_print(F("b4 "));
    flush_rx_serial();
    if ( ! enter_command_mode_with_retries() ) { 
        debug_serial_println(F("failed\n"));
        return -1; 
    }
    debug_serial_println(F(" after"));

    //TODO - do we need these timeouts? 

    delay(1500); // give a just booted radio tim to be ready.

    Serial.write("\r\n"); // after +++ we need to clear the line before we set AT commands
    Serial.flush(); // output buffer flush
    delay(500); // give a just booted radio tim to be ready.

    flush_rx_serial();

    //  read AT params from radio , and write to a file in spiffs.
    String prefix = "AT";
    if (filename == RFD_REM_PAR ) {prefix = "RT";}

    // untested- implement factory_reset_first boolean
    if ( factory_reset_first ) { 
        debug_serial_print(F("attempting factory reset.... &F and &W ... "));

        String factorycmd = prefix+"&F\r\n";
        debug_serial_print(factorycmd);
        Serial.write(factorycmd.c_str());
        Serial.flush(); // output buffer flush
        bool b = SmartSerial->expect("OK",200); 
        (void)b; // compiler unsed variable warning otherwise 
        flush_rx_serial();

        String factorycmd2 = prefix+"&W\r\n"; 
        debug_serial_print(factorycmd);
        Serial.write(factorycmd2.c_str());
        Serial.flush(); // output buffer flush
        bool b2 = SmartSerial->expect("OK",200); 
        (void)b2; // compiler unsed variable warning otherwise 
        flush_rx_serial();

        debug_serial_print(F("...attempted factory reset.\r\n"));
    }

    if (filename == RFD_LOC_PAR ) { 

        r900x_savesingle_param_and_verify_more("AT", PARAM_STATLED_STR, "1", false);

        int param_save_counter = 0;
        do {
            // save params AND take out of command mode via a quick reboot
            String savecmd = prefix+"&W\r\n"; // "AT&W\r\n
            debug_serial_print(savecmd); // debug only.
            Serial.write(savecmd.c_str());
            Serial.flush(); // output buffer flush

            bool ok = SmartSerial->expect("OK",500); 
            if ( ok ) { 
                debug_serial_println(F("GOT OK Response from radio.\n"));
                param_save_counter = 5;
            } else { 
                debug_serial_println(F("FALED-TO-GET OK Response from radio. - retrying:\n"));
                delay(100); // might help by delaying the retry a bit.
                param_save_counter++;

                // if 5 retries exceeded, return error.
                debug_serial_println(F("RETURN:no response to param save request - 5 retries exceeded")); 
            }
        } while (param_save_counter < 5);    

        flush_rx_serial();    
    }

    // TODO get version info here from local and/or remote modem with an AT command

    // now get params list ATI5 or RTI5 as needed 
    String cmd = prefix+"I5\r\n";
    Serial.write(cmd.c_str());
    Serial.flush(); // output buffer flush
    //debug_serial_print(F("----------------------------------------------"));
    String data = SmartSerial->expect_s("HYSTERESIS_RSSI",1000); 
    data += SmartSerial->expect_s("\r\n",200);
    while (Serial.available() ) { char t = Serial.read();  data += t; } // flush read buffer upto this point.
    debug_serial_print(data);
    //debug_serial_print(F("----------------------------------------------"));

    // count number of lines in output, and return it as result.
    unsigned int linecount = 0;
    for ( unsigned int c = 0 ; c < data.length(); c++ ) { 
        if ( data.charAt(c) == '\n' ) { linecount++; }
    }

    // as user experience uses the SPIFFS .txt to render the html page, we cleanup an old one if we've been asked
    // to get fresh params, even if we cant replace it, as the *absense* of it means the remote radio is 
	// no longer connected.
    SPIFFS.remove(filename); 

    
    // now write params to spiffs, for user record:
    if ( data.length() > 300 ) { // typical file length is around 400chars 
        f = SPIFFS.open(filename, "w");
        data.trim();
        data += "\r\n";
        f.print(data);  //actual parm data.

        // tack encryption key onto the end of the param file, if it exists, instead of read from remote radio
		// as we can't do that right now. - TODO.
        File e = SPIFFS.open(RFD_ENC_KEY, "r");
        String estr = "&E:ENCRYPTION_KEY="+e.readString();// entire file, includes /r/n on end.
        
        if (estr.length() > 30 && e) { // basic check, file should exist and have at least 30 bytes in it to be plausible
          f.print(estr);
        }
        e.close(); 

        f.close();
    } else { 
        debug_serial_println(F("didn't write param file, as it contained insufficient data"));
    }

    String vercmd = prefix+"I\r\n";  //ATI or RTI
    debug_serial_print(vercmd.c_str()); // for debug only
    Serial.write(vercmd.c_str());
    Serial.flush(); // output buffer flush

    String vers; //starts with this...
    bool ok = SmartSerial->expect(rfd_sik_sig,1500);  // we really want to see 'RFD SiK' here, 

    if ( ok ) { 
        vers = rfd_sik_sig;
        delay(250);
        debug_serial_println(F("\tGOT SiK Response from radio."));

        // this line *may* have started with 'RFD SiK' ( we matched on the SiK above), and end with '2.65 on RFD900X R1.3' 
        while (Serial.available() ) { 
            char t = Serial.read();  
            debug_serial_print(t);
            vers += t; 
        } // flush read buffer upto this point, displaying it for posterity ( it has version info )

        debug_serial_print(F("VERSION STRING FOUND:"));
        debug_serial_println(vers);

        // save version string to a file for later use by the webserver to present to the user.
        String vf = RFD_LOC_VER;
        if (filename == RFD_REM_PAR ) {vf = RFD_REM_VER;}
        File v = SPIFFS.open(vf, "w"); 
        v.print(vers);
        v.close();

    } else {
        debug_serial_println(F("\tFAILED-TO-GET SiK Response from radio.\n"));
    } 
	
    delay(1000); // time for local and remote to sync and RT values to populate.

    debug_serial_print(F("ATZ/RTZ ensures RFD900x leaves command mode "));
    cmd = prefix+"Z\r";
    Serial.write(cmd.c_str()); // reboot radio to restore non-command mode.
    Serial.flush(); // output buffer flush
    delay(200); 

    debug_serial_println(F("r900x_getparams()- END\n"));

    return linecount;
}

// Converts the RSSI figure incomming from a MAV message ID 109
// to a figure that represents link quality in percentage
uint8_t r900x_rssi_percentage(uint8_t mav_rssi_109) {
    uint16_t res;
    
    if (mav_rssi_109 == 255) // MAV value that corresponsds to invalid/unknown
    {
        res = 0;
    } else if (mav_rssi_109 >= r9x_sensitivity + 20) {
        res = 99;
    } else {
        res = 100 * (mav_rssi_109 - r9x_sensitivity);
        res /= 20;
    }

    //debug_serial_println("rssi_reported:" + String(mav_rssi_109));
    delay(0);
    //debug_serial_println("sensitivity:" + String(r9x_sensitivity));
    delay(0);
    //debug_serial_println("rssi_per_calc:" + String(res));
    delay(0);

    return res;
}

void r900x_setup(bool reflash) { // if true. it will attempt to reflash ( and factory defaults), if false it will go through the motions.

    debug_serial_println(F("r900x_setup(...)\n"));
    delay(1000);  // allow time for 900x radio to complete its startup.? 


    set_led_state(0);

    if ( SPIFFS.begin() ) { 
      debug_serial_println(F("spiffs started\n"));
    } else { 
      debug_serial_println(F("spiffs FAILED to start\n"));
    }

    //Format File System if it doesn't at least have an index.htm file on it.
    if (!SPIFFS.exists("/index.htm")) {
        debug_serial_println(F("SPIFFS File System Format started...."));
        SPIFFS.format();
        debug_serial_println(F("...SPIFFS File System Format Done."));
    }
    else
    {
        debug_serial_println(F("SPIFFS File System left as-is."));
    }

    // debug only, list files and their size-on-disk
    debug_serial_println(F("List of files in SPIFFs:"));
    Dir dir = SPIFFS.openDir(""); //read all files
    while (dir.next()) {
      debug_serial_print(dir.fileName()); debug_serial_print(F(" -> "));
      File f = dir.openFile("r");
      debug_serial_println(f.size());
      f.close();
    }

    // Store pseudo-sensitivity value in the RAM to calculate RSSI
    // Pseudo-sensitivity is calculated based on RSSI value from MAV message #109:
    // pRSSI = 2*sensitivity + 304
    // Sensitivity values can be found in the RFD900x datasheet
    File f = SPIFFS.open(RFD_LOC_PAR,"r");
    if (f != NULL) {
        bool as_found = false;
        while (f.available()) {
            String line = f.readStringUntil('\n');
            int res = line.indexOf("AIR_SPEED");
            if (res >= 0) {
                int airspeed = line.substring(res + 10).toInt();
                switch (airspeed) {
                    case 12: r9x_sensitivity = 82; break; // -111dBm
                    case 56: 
                    case 64: 
                    case 100: r9x_sensitivity = 94; break; // -105dBm
                    case 125: r9x_sensitivity = 96; break; // -104dbm
                    case 200: 
                    case 224: 
                    case 500: r9x_sensitivity = 116; break; // -94dBm
                    case 750: r9x_sensitivity = 126; break; // -89dBm
                    default: r9x_sensitivity = 94; break; // Defaults to -105dBm seen on 64kpbs datarates
                }
                as_found = true;
                debug_serial_println("AS found:"+String(airspeed));
                break;
            }
        }
    }
    debug_serial_println("Sensitivity:" +String(r9x_sensitivity));

    int baudrate = getWorld()->getParameters()->getUartBaudRate();
    debug_serial_println("Loading baud from FLASH: " + String(baudrate));
    debug_serial_println("Serial.begin("+String(baudrate)+");");
    Serial.begin(baudrate); // // get params from modem with command-mode, without talking ot the bootloader, at stock firmware baud rate.

    f = SPIFFS.open(BOOTLOADERNAME, "r");

    // if we've got anything available to try a reflash, must be a .bin
    if ( reflash) { 
        if ( ! f ) { 
            f.close();
            //debug_serial_println("flashing with .ok firrmware....\n");
            //f = SPIFFS.open(BOOTLOADERCOMPLETE, "r");
            f = SPIFFS.open(BOOTLOADERNAME, "r"); // retry opening .bin file, just in case it was intermittent.
        }
    }

    if ( ! f ) {  // did we open this file ok, ie does it exist? 
        debug_serial_println(F("no firmware .bin available to program, skipping reflash.\n"));
        return;
    }


    if (( f.size() > 0) and (f.size() < 90000 )) { 
        debug_serial_println(F("incomplete or too-small firmware for 900x to program ( < 90k bytes ), deleting corrupted file and skipping reflash.\n"));
        SPIFFS.remove(BOOTLOADERNAME); 
        return;   
    } 

    //All possible baudrates
    const int baud_list [] = {57600,115200,9600,19200,38400,230400,460800,1000000,4800,2400,1200};
    const int baud_list_size = sizeof(baud_list)/sizeof(baud_list[0]);

    // first try at the default baud rate...
    int result = -1; // returns 1,2,3,4 or 5 on some sort of success

    for (int r=0; r < 3; r++){

        toggle_led_state();

        //TODO - can we do Hardware flow control? 
        // https://www.esp8266.com/viewtopic.php?f=27&t=8560
        // maybe not with the arduino IDE, but maybe yes with the espressif SDK...? 

        // possible impl:
        // const int CTSPin = 13; // GPIO13 for CTS input
        // pinMode(CTSPin, FUNCTION_4); // make pin U0CTS
        // U0C0 |= UCTXHFE; //add this sentense to add a tx flow control via MTCK( CTS )

        // other notes:
        // SET_PERI_REG_MASK( UART_CONF0(uart_no),UART_TX_FLOW_EN); //add this sentense to add a tx flow control via MTCK( CTS )
        // or U0C0 |= UCTXHFE; //add this sentense to add a tx flow control via MTCK( CTS )
        // https://github.com/esp8266/Arduino/blob/master/cores/esp8266/esp8266_peri.h#L189
        //
        //As for the GPIOs, when flow control is enabled:
        //GPIO13	= U0CTS
        //GPIO15	= U0RTS
        // this last link uses the RTOS SDK, not the arduino one, so isn't 100% accurate...
        // http://forgetfullbrain.blogspot.com/2015/08/uart-sending-and-receiving-data-using.html

        debug_serial_println(F("Serial.begin(57600);"));
        Serial.begin(57600);

        // first try to communicate with the bootloader, if possible....
        int ok = r900x_booloader_mode_sync();
        if ( ok == 1 ) { debug_serial_println(F("got boot-loader sync(ChipID)"));  
            result = 1; break; } else
        if ( ok == 2 ) { debug_serial_println(F("got boot-loader sync(UPLOAD)"));  
            result = 2; break; }

        if ( got_hex == false ) { // hack buzz temp disable, r-enable me.
            // Try with the saved baudrate first
            debug_serial_println("Serial.begin("+String(baudrate)+");");
            Serial.begin(baudrate);

            // if that doesn't work, try to communicate with the radio firmware, and put it into AT mode...
            ok = r900x_command_mode_sync();
            if (ok) { 
                debug_serial_println(F("got command-mode sync"));  
                result = 9;
            } else {
                // Attempt entering command mode with every single baudrate
                for (int i = 0; i < baud_list_size; i++) {
                    if (baud_list[i] != baudrate) {
                        debug_serial_println("Serial.begin("+String(baud_list[i])+");");
                        Serial.begin(baud_list[i]);

                        // if that doesn't work, try to communicate with the radio firmware, and put it into AT mode...
                        ok = r900x_command_mode_sync();
                        if (ok) { 
                            debug_serial_println(F("got command-mode sync"));  
                            result = 9;
                                
                            //Update baudrate on Flash
                            debug_serial_println("Saving serial to FLASH: " + String(baud_list[i]));
                            getWorld()->getParameters()->setUartBaudRate(baud_list[i]);
                            getWorld()->getParameters()->saveAllToEeprom();
                            
                            break; 
                        }
                    }
                }
                
                // Failed to enter command mode 
                if (!ok) break;
            }
        }
     
        // result= -1 might mean our modem was maybe  already stuck in bootloader mode to start with.., we can try to recover that too... 
        debug_serial_print("result:"); debug_serial_println(result);
        delay(200);

        //debug_serial_print("r900x_upload anyway attempt....\n--#");
        flush_rx_serial();
        debug_serial_println("#--");

        // if we already saw ChipID or UPLOAD output, don't try to do 
        if ( result == 9 ) {
            Serial.begin(57600);
            //Serial.write("U"); // AUTO BAUD RATE CODE EXPECTS THIS As the first byte sent to the bootloader, not even \r or \n should be sent earlier
            Serial.write("\r\n");
            Serial.flush();

            bool ok = SmartSerial->expect("RFD900xSub:2",2000);  // response to 'U' is the long string including chipid
            // todo handle this return value. 
            //for now just to stop compilter warning:
            ok = !ok;

            delay(200);

            flush_rx_serial();
        }

        // UPLOAD COMMAND EXPECTS 'Ready' string
        debug_serial_println(F("\t\tsending 'UPLOAD'\r\n"));
        //
        Serial.write("UPLOAD\r\n");
        Serial.flush(); // output buffer flush

        // we really MUST see Ready here
        if (SmartSerial->expect("Ready",3000)) 
        {
            if (SmartSerial->expect("\r\n",1000)) 
            { 
                // and then some \r\n precisely.
                debug_serial_println(F("\t\tGOT UPLOAD/Ready Response from radio.\n"));
                flush_rx_serial();

                //  xmodem stuff...
                debug_serial_println(F("bootloader handshake done"));

                if ( reflash ) { 
                    debug_serial_println(F("\t\tXMODEM SENDFILE BEGAN\n"));
                    int xok = xmodem.sendFile(f);

                    if (xok == -1) {
                        debug_serial_println(F("ERROR! gave up on xmodem-reflash, sorry."));  
                    } else if (xok == 1) {
                        debug_serial_println(F("renamed firmware file after successful flash ( file ends in .ok now)"));

                        // after flashing successfully from the .bin file, rename it
                        SPIFFS.remove(BOOTLOADERCOMPLETE); // cleanup incase an old one is still there. 
                        SPIFFS.rename(BOOTLOADERNAME, BOOTLOADERCOMPLETE); // after a successful upload to the 900x radio, rename it out of the 

                        debug_serial_println(F("Booting new firmware"));
                        Serial.write("BOOTNEW\r");
                        Serial.flush();
                        debug_serial_println(F("\t\tBOOTNEW\r\n"));     
                        break;
                    }                
                    debug_serial_println(F("\t\tXMODEM SENDFILE ENDED\n"));
                } else { 
                    debug_serial_println(F("\t\tXSKIPPING XMODEM REFLASH BECAUSE OF 'reflash' false.\n"));
                }                                  
            } else { 
                debug_serial_println(F("\t\tFAILED-TO-GET UPLOAD/Ready Response from radio.\n"));
                //return false;
                //result = 6;                 
            } 
        } else {
            debug_serial_println(F("Error! Didn't receive Ready"));
        }      

        debug_serial_println(F("Booting firmware"));
        Serial.write("BOOTNEW\r");
        Serial.flush();
        debug_serial_println(F("\t\tBOOTNEW\r\n")); 

        flush_rx_serial();
    }

    baudrate = getWorld()->getParameters()->getUartBaudRate();
    debug_serial_print("Loading baud from FLASH: " + String(baudrate));
    debug_serial_println("Serial.begin("+String(baudrate)+");");
    Serial.begin(baudrate); // // get params from modem with command-mode, without talking ot the bootloader, at stock firmware baud rate.
    flush_rx_serial();
    Serial.flush();

    //we put a AT&F here to factory-reset the modem after the reflash and before we get params from it
    
    r900x_getparams(RFD_REM_PAR,true);  // true = reset to factory defaults before reading params
    r900x_getparams(RFD_LOC_PAR,true);         // true = reset to factory defaults before reading params

    f.close();

} 

void r900x_attempt_factory_reset(void) {
    if (enter_command_mode_with_retries())
    {
        for (uint8_t tries = 0; tries < 5; tries++) {
            bool success = false;
            debug_serial_println(F("attempting AT&F"));
            String factorycmd = "AT&F\r\n";
            Serial.write(factorycmd.c_str());
            Serial.flush(); // output buffer flush
            if (SmartSerial->expect("OK",250)) {
                flush_rx_serial();

                for (uint8_t tries_w = 0; tries_w < 5; tries_w++) {
                    debug_serial_println(F("attempting AT&W"));
                    String factorycmd2 = "AT&W\r\n"; 
                    Serial.write(factorycmd2.c_str());
                    Serial.flush(); // output buffer flush
                    if (SmartSerial->expect("OK",250)) {
                        flush_rx_serial();
                        debug_serial_println(F("Sending ATZ"));
                        String factorycmd2 = "ATZ\r\n"; 
                        Serial.write(factorycmd2.c_str());
                        Serial.flush(); // output buffer flush
                        flush_rx_serial();
                        success = true;
                        break;
                    }
                    flush_rx_serial();
                }
                if (success) break;
            }            
        } 
    }
}


#pragma once

#include "txmod_debug.h"
#include "hwdefs.h"

class MySerial { 
public:
  bool _serial_connected = false;

  uint8_t rawserialbytes[255];

  unsigned long last_ms; 

  MySerial(HardwareSerial *serial) {
  }
  void begin() { 
      //Serial9xPri->begin(4800, SERIAL_PARAM1, SERIAL1_RXPIN, SERIAL1_TXPIN);
      // delay(1000); // time to start up.

      // put_into_at_mode();

      debug_serial_println("SmartSerial begin() done.");
        
      _serial_connected = true;
  }

 // similar to expect() except returns the complete raw string-data parsesd/found on success, and empty string on fail.
  String expect_s( String lookfor, uint32_t wait_ms ) { 

    String return_val = "";
    last_ms = millis();
    //bool found = false;
    // look for response, for max X milli seconds
    debug_serial_println("s_waiting for s:"+ lookfor);
    unsigned int offset = 0;
    bool finding = false; // are we part-way through a string match? 

   // debug_serial_print("-->");
    int saw_any_data = 0;
    while ((last_ms+wait_ms) > millis() )     {
        if ( Serial9xPri.available() ) {
            char c = Serial9xPri.read();
            saw_any_data++;
           // debug_serial_print("Z:"); // debug only
           // debug_serial_println(String(c)); // debug only
            return_val += c;
            if ( c == lookfor.charAt(offset) ) {  // next char we got is the next char we expected, so move forward a char and keep looking.
                offset++;
                finding = true;
            } else {  // we got a char that isn't the next one we wanted.
                finding = false;
                offset = 0;
            }
            if (( finding==true ) && ( offset >= lookfor.length() )) { // we found the entire string, return immediately, dont wait for any more serial data or timeout
                debug_serial_print("<--");
                debug_serial_print(" found s:"+ return_val); 
                debug_serial_print(" with len:"); debug_serial_print(offset);
                debug_serial_print(" scanned:"); debug_serial_print(saw_any_data);
                debug_serial_print(" ms:"); debug_serial_println(millis()-last_ms);
                return return_val;
            } 
        }
    }
    debug_serial_print("!--");
    debug_serial_print(" NOT-found s:"+ lookfor); 
    debug_serial_print(" scanned:"); debug_serial_print(saw_any_data);
    debug_serial_print(" ms:"); debug_serial_println(millis()-last_ms);
    return return_val;
  }

  bool expect( String lookfor, uint32_t wait_ms ) { 

    last_ms = millis();
    //bool found = false;
    // look for response, for max X milli seconds
    debug_serial_print("e_waiting for s:"+ lookfor);
    unsigned int offset = 0;
    bool finding = false; // are we part-way through a string match? 
    int rawcount = 0;
    debug_serial_print(" -->");
    while ((last_ms+wait_ms) > millis() )     {
        if ( Serial9xPri.available() ) {
            char c = Serial9xPri.read();  rawcount++;
            debug_serial_print(c); // debug only
            if ( c == lookfor.charAt(offset) ) {  // next char we got is the next char we expected, so move forward a char and keep looking.
                offset++;
                finding = true;
            } else {  // we got a char that isn't the next one we wanted.
                finding = false;
                offset = 0;
            }
            if (( finding==true ) && ( offset >= lookfor.length() )) { // we found the entire string, return immediately, dont wait for any more serial data or timeout
                debug_serial_print("<--");
                debug_serial_print(" found s:"+ lookfor); 
                debug_serial_print(" with len:"); debug_serial_print(offset);
                debug_serial_print(" ms:"); debug_serial_println(millis()-last_ms);
                return true;
            } 
        }
    }
    debug_serial_print("<--");
    debug_serial_print(" raw bytes read:"); debug_serial_print(rawcount);
    debug_serial_print(" timeout ms:"); debug_serial_println(millis()-last_ms);
    return false;
  }


  // return negative vals for error, positive vals to say which match was successful
  // this is essentially the same code as expect() function, but can handle looking for upto *three* different strings concurrently.
  int expect_multi3( String lookfor, String lookfor2, String lookfor3, uint32_t wait_ms ) { 

    last_ms = millis();
    //bool found = false;
    // look for response, for max X milli seconds
    debug_serial_print("wait for s1:"+ lookfor);
    debug_serial_print(" wait for s2:"+ lookfor2);
    debug_serial_print(" wait for s3:"+ lookfor3);
    unsigned int offset = 0;
    unsigned int offset2 = 0;
    unsigned int offset3 = 0;
    bool finding = false; // are we part-way through a string match? 
    bool finding2 = false; // are we part-way through a string match? 
    bool finding3 = false; // are we part-way through a string match? 

    debug_serial_print("-->");
    while ((last_ms+wait_ms) > millis() )     {
        if ( Serial9xPri.available() ) {
            char c = Serial9xPri.read();
            debug_serial_print(c); // debug only

            if ( c == lookfor.charAt(offset) ) {  // next char we got is the next char we expected, so move forward a char and keep looking.
                offset++;
                finding = true;
            } else {  // we got a char that isn't the next one we wanted.
                finding = false;
                offset = 0;
            }

            if ( c == lookfor2.charAt(offset2) ) {  // next char we got is the next char we expected, so move forward a char and keep looking.
                offset2++;
                finding2 = true;
            } else {  // we got a char that isn't the next one we wanted.
                finding2 = false;
                offset2 = 0;
            }

            if ( c == lookfor3.charAt(offset3) ) {  // next char we got is the next char we expected, so move forward a char and keep looking.
                offset3++;
                finding3 = true;
            } else {  // we got a char that isn't the next one we wanted.
                finding3 = false;
                offset3 = 0;
            }

            if (( finding==true ) && ( offset >= lookfor.length() )) { // we found the entire string, return immediately, dont wait for any more serial data or timeout
                debug_serial_print("<1--");
                debug_serial_print(" found s1:"+ lookfor); 
                debug_serial_print(" with l1:"); debug_serial_print(offset);
                debug_serial_print(" ms:"); debug_serial_println(millis()-last_ms);
                return 1;
            } 

            if (( finding2==true ) && ( offset2 >= lookfor2.length() )) { // we found the entire string, return immediately, dont wait for any more serial data or timeout
                debug_serial_print(" <2--");
                debug_serial_print(" found s2:"+ lookfor2); 
                debug_serial_print(" with l2:"); debug_serial_print(offset);
                debug_serial_print(" ms:"); debug_serial_println(millis()-last_ms);
                return 2;
            } 

            if (( finding3==true ) && ( offset3 >= lookfor3.length() )) { // we found the entire string, return immediately, dont wait for any more serial data or timeout
                debug_serial_print(" <3--");
                debug_serial_print(" found s3:"+ lookfor3); 
                debug_serial_print(" with l3:"); debug_serial_print(offset);
                debug_serial_print(" ms:"); debug_serial_println(millis()-last_ms);
                return 3;
            } 
        }
    }
    debug_serial_print("<--");
    debug_serial_print(" timeout ms:"); debug_serial_println(millis()-last_ms);
    return -1;
  }

};



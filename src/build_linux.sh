#!/bin/bash
SRCFILES=`./srcfiles.sh`
g++ -g -std=c++17 -I . -I ./HAL/Linux/submodules/EpoxyDuino/cores/epoxy -I ./HAL/Linux/submodules/EpoxyDuino/libraries/EpoxyFS/src -I ./HAL/Linux -I ../.pio/libdeps/esp12e/EspSoftwareSerial_ID168/src -I "../.pio/libdeps/esp12e/MAVLink v2 C library_ID6412" -I ../.pio/libdeps/esp12e/CircularBuffer_ID1796 -I ../lib/ESP8266WebServer/src -I ./HAL/Linux/submodules/libb64/include -I ../lib/ESP8266HTTPUpdateServer -I ./HAL/Linux/submodules/RFDProxy -Wno-address-of-packed-member -DDEBUG_USE_SW_SERIAL -DEPOXY_CORE_ESP8266 -DESP8266 $SRCFILES -L/usr/lib -o txmod2

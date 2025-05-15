#ifndef HWDEFS_H
#define HWDEFS_H    

// 3V3	  3.3,  V power supply
// EN  CHIP_PU, Reset
// VP	GPIO36, ADC1_CH0, S_VP
// VN	GPIO39, ADC1_CH3, S_VN
// IO34	GPIO34, ADC1_CH6, VDET_1                                                
// IO35	GPIO35, ADC1_CH7, VDET_2                                                
// IO32	GPIO32, ADC1_CH4, TOUCH_CH9, XTAL_32K_P
// IO33	GPIO33, ADC1_CH5, TOUCH_CH8, XTAL_32K_N                                 
// IO25	GPIO25, ADC1_CH8, DAC_1	                                                
// IO26	GPIO26, ADC2_CH9, DAC_2	                                                
// IO27	GPIO27, ADC2_CH7, TOUCH_CH7	                                            
// IO14	GPIO14, ADC2_CH6, TOUCH_CH6, MTMS                                       // FTDI VCC
// IO12	GPIO12, ADC2_CH5, TOUCH_CH5, MTDI                                       // note this pin is used for the bootloader, so don't use it
// GND	Ground	                                                                // FTDI GND
// IO13	GPIO13, ADC2_CH4, TOUCH_CH4, MTCK                                       Status LED 
// D2	GPIO9 , SD_D2 U1RXD	                                                    ! Unusable flash memory !
// D3	GPIO10, SD_D3 U1TXD                                                     ! Unusable flash memory !
// CMD	GPIO11, SD_CMD U1RTS                                                         ! Unusable flash memory !
// 5V	5 V power supply
 

// GND	  Ground
// IO23  GPIO23,                                                                U1RX
// IO22  GPIO22, U0RTS                                                          U1TX
// TX	  GPIO1, U0TXD                                                          U0TXD AUX_RX
// RX	  GPIO3, U0RXD                                                          U0RXD AUX_TX
// IO21  GPIO21                                                                 LED
// GND	  Ground
// IO19  GPIO19, U0CTS                                                          RESET
// IO18  GPIO18                                                                 AUX_RTS (U1CTS)
// IO5	  GPIO5                                                                 AUX_RX (U1TXD)
// IO17  GPIO17 3, U2TXD                                                        U1_TXD MAIN_RX
// IO16  GPIO16 3, U2RXD                                                        U1_RXD MAIN_TX
// IO4	  GPIO4, ADC2_CH0, TOUCH_CH0                                            U2_TXD
// IO0	  GPIO0, ADC2_CH1, TOUCH_CH1, Boot                                      // boot pin do not use 
// IO2	  GPIO2, ADC2_CH2, TOUCH_CH2                                            // caueses programming failure
// IO15  GPIO15, ADC2_CH3, TOUCH_CH3, MTDO                                      DBG_TX
// D1    GPIO8, SD_D1 U2CTS                                                     ! Unusable flash memory !   
// D0    GPIO7, SD_D0 U2RTS                                                     ! Unusable flash memory !   
// CLK   GPIO6, SD_CLK U1CTS                                                         ! Unusable flash memory !

#if defined(ESP32) 
#define tx0Pin 1                                                                // TX0 pin
#define rx0Pin 3                                                                // RX0 pin
#define cts0Pin -1//19                                                              // CTS0 pin
#define rts0Pin -1//22                                                              // RTS0 pin
#define tx1Pin 22                                                               // TX1 pin
#define rx1Pin 23                                                               // RX1 pin
#define cts1Pin -1                                                              // CTS1 pin
#define rts1Pin -1                                                              // RTS1 pin
#define tx2Pin 17                                                                // TX2 pin
#define rx2Pin 16                                                               // RX2 pin
#define NomDbgTxPin 15                                                          // TX Debug pin nominal location

#define StatLEDPin 13                                                           // Status LED pin
#define LEDGPIO StatLEDPin

#define LED 21
#define RESETGPIO 19


#define ENABLE_DEBUG 1                                                          // Enable debug output for main.cpp(dhcp),mavesp8266.cpp/h (mav log)
#define DEBUG_ENABLE 1                                                          // Enable debug output for rfd900x.cpp (rfd900x,xmodem,sport,smartserial,main)
#define DEBUG 1                                                                 // Enable debug output for mavesp8266_parameters.cpp

#define ANYDEBUG (ENABLE_DEBUG || DEBUG_ENABLE || DEBUG)                        // any debug output enabled

#define DEBUG_WEB 0

#include <HardwareSerial.h>

extern HardwareSerial Serial9xPri;                                              // Serial 1
extern HardwareSerial Serial9xAux;                                              // Serial 0
extern HardwareSerial frSerial;                                                 // Serial 2
#define DEBUG_USE_SW_SERIAL 1

#if DEBUG_USE_SW_SERIAL
#include <SoftwareSerial.h>
extern SoftwareSerial dbgSer;
#define txDbgPin NomDbgTxPin
#define DBGBAUD 57600
#else
extern HardwareSerial dbgSer;                                                   // Serial 0
#define dbgSerNo 0
#define txDbgPin tx0Pin                                                         // TX Debug pin
#define DBGBAUD 115200
#endif
#define rxDbgPin -1                                                             // RX Debug pin

#define AUXUART 2
#define txAuxPin tx2Pin                                                         // TX Aux pin 
#define rxAuxPin rx2Pin                                                         // RX Aux pin
#define ctsAuxPin cts2Pin                                                       // CTS Aux pin
#define rtsAuxPin rts2Pin                                                       // RTS Aux pin

#define MDMUART 1
#define txMdmPin tx1Pin                                                         // TX MDM pin
#define rxMdmPin rx1Pin                                                         // RX MDM pin
#define ctsMdmPin cts1Pin                                                       // CTS MDM pin
#define rtsMdmPin rts1Pin                                                       // RTS MDM pin

#define FRUART 0
#define txFrPin tx0Pin                                                          // TX FR pin
#define rxFrPin -1

#endif

#endif
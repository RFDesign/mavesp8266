#include "hwdefs.h"
#include "txmod_debug.h"

#define DEBUG_SERIAL dbgSer

#if ANYDEBUG
#if DEBUG_USE_SW_SERIAL
SoftwareSerial dbgSer;
#elif DEBUG_WEB
#include "webdebug.h"
#else 
HardwareSerial dbgSer(dbgSerNo);
#endif
#endif

void debug_init()
{
    static bool initialized = false;
    if (initialized) return;
    initialized = true;
    #if ANYDEBUG
    #if DEBUG_USE_SW_SERIAL
        dbgSer.begin(DBGBAUD,SWSERIAL_8N1,rxDbgPin,txDbgPin,false);               // note SwSerial cannot work above 57600 baud
        dbgSer.enableIntTx(true);                                               // Enable interrupts for SWSerial tx
        dbgSer.println(F("[MSG] initd swSer output"));
    #elif DEBUG_WEB
        webdebug_init();
    #else
        dbgSer.begin(DBGBAUD,SERIAL_8N1,rxDbgPin,txDbgPin,false);
        dbgSer.println(F("[MSG] initd Serial output"));
        dbgSer.setDebugOutput(true);
    #endif
    #endif
}

void debug_println(String line)
{
#if DEBUG_WEB
    char buff[64];
    memset(buff, 0, sizeof(buff));
    line += "\n";
    line.toCharArray(buff, sizeof(buff));
    uint8_t len = line.length() > sizeof(buff) ? sizeof(buff) : line.length();
    webdebug_publish(buff, len);
#else
    DEBUG_SERIAL.println(line);
#endif
}

void debug_print(String str)
{
#if DEBUG_WEB
    char buff[64];
    memset(buff, 0, sizeof(buff));
    str.toCharArray(buff, sizeof(buff));
    uint8_t len = str.length() > sizeof(buff) ? sizeof(buff) : str.length();
    webdebug_publish(buff, len);
#else
    DEBUG_SERIAL.print(str);
#endif    
}

void debug_flush()
{
#if DEBUG_WEB 
#else
    DEBUG_SERIAL.flush();
#endif    
}
#include <SoftwareSerial.h>
#include "txmod_debug.h"

#ifdef DEBUG_USE_SW_SERIAL
#define DEBUG_SERIAL swSer
SoftwareSerial swSer;
#elif defined DEBUG_WEB
#include "webdebug.h"
#else
#define DEBUG_SERIAL Serial1
#endif

void debug_init()
{
    #if not defined DEBUG_DISABLE
    #ifdef DEBUG_USE_SW_SERIAL
        swSer.begin(115200,SWSERIAL_8N1,14,16,false);
        swSer.println(F("[MSG] initd swSer output"));
    #elif defined DEBUG_WEB
        webdebug_init();
    #else
        Serial1.begin(115200);
        Serial1.println(F("[MSG] initd Serial output"));
        //Serial1.setDebugOutput(true);
    #endif
    #endif
}

void debug_println(String line)
{
#ifdef TXMOD_HAL
	printf("%s\n", line.c_str());
#else
	#ifdef DEBUG_WEB
		char buff[64];
		memset(buff, 0, sizeof(buff));
		line += "\n";
		line.toCharArray(buff, sizeof(buff));
		uint8_t len = line.length() > sizeof(buff) ? sizeof(buff) : line.length();
		webdebug_publish(buff, len);
	#else
		DEBUG_SERIAL.println(line);
	#endif
#endif
}

void debug_print(String str)
{
#ifdef TXMOD_HAL
	printf("%s", str.c_str());
#else
	#ifdef DEBUG_WEB
		char buff[64];
		memset(buff, 0, sizeof(buff));
		str.toCharArray(buff, sizeof(buff));
		uint8_t len = str.length() > sizeof(buff) ? sizeof(buff) : str.length();
		webdebug_publish(buff, len);
	#else
		DEBUG_SERIAL.print(str);
	#endif
#endif
}

void debug_flush()
{
#ifdef DEBUG_WEB
    
#else
    DEBUG_SERIAL.flush();
#endif    
}

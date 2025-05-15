#ifndef DEBUG_H
#define DEBUG_H

#include <Arduino.h>

void debug_init();
void debug_println(String line);
void debug_print(String str);
void debug_flush();

#define debug_crit_println(X) \
    debug_serial_println(String(X)); \
    debug_flush()
#define debug_crit_print(X) \
    debug_serial_print(String(X)); \
    debug_flush()

#if !DEBUG_ENABLE
#define debug_serial_println(X)
#define debug_serial_print(X)
#else
#define debug_serial_println(X) debug_println(String(X))
#define debug_serial_print(X) debug_print(String(X))
#endif

#endif
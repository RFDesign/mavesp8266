/*
 * debug.cpp
 *
 *  Created on: 2 Apr 2026
 *      Author: snh
 */

#include "debug.hpp"
#include "submodules/RFDProxy/time/monotonic.hpp"
#include <stdarg.h>
#include <stdio.h>

void Debug_Printf(const char *fmt, ...)
{
	va_list args;

	printf("%llu:  ", (unsigned long long)rfdproxy::time::GetMonotonicTimeInMilliseconds());

	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	fflush(stdout);
}


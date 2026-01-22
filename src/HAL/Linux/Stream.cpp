/*
 * Stream.cpp
 *
 *  Created on: 26 Nov 2025
 *      Author: snh
 */

#include <string.h>
#include "Arduino.h"
#include "submodules/libb64/include/libb64/cencode.h"


void Stream::write(char x)
{

}

void Stream::setTimeout(int x)
{

}

size_t Stream::readBytes(char *Dest, int Length)
{
	return Length;
}

HardwareSerial::HardwareSerial(int PortNumber)
{

}

void MD5Builder::begin(void)
{

}

void MD5Builder::add(String x)
{

}

void MD5Builder::calculate(void)
{

}

String MD5Builder::toString(void)
{
	return String("");
}

void pinMode(int a,  int b)
{

}

void digitalWrite(int a, int b)
{

}

bool digitalRead(int a)
{
	return false;
}

void interrupts(void)
{

}

void noInterrupts(void)
{

}

void attachInterrupt(int, void (*)(void), int)
{

}

bool isDigit(char c)
{
	return c >= '0' && c <= '9';
}

size_t ets_vsnprintf(char *Buff, int n, const char * fmt, va_list &l)
{
	return vsnprintf(Buff, n, fmt, l);
}

size_t base64_encode_expected_len(size_t n)
{
	base64_encodestate State;

	base64_init_encodestate(&State);

	return base64_encode_length(n, &State);
}

size_t base64_encode_chars(char *In, size_t n, char *Out)
{
	base64_encodestate State;

	base64_init_encodestate(&State);

	size_t Result = base64_encode_block(In, n, Out, &State);
	Result += base64_encode_blockend(Out, &State);
	return Result;
}

uint32_t GetRandom32(void)
{
	return random();
}

void yield(void)
{

}

size_t strlen_P(const void *s)
{
	return strlen((const char *)s);
}

void memccpy_P(void *Dest, void *Src, int QTY, size_t Size)
{
	memcpy(Dest, Src, QTY * Size);
}



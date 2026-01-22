/*
 * SoftwareSerial.cpp
 *
 *  Created on: 18 Dec 2025
 *      Author: snh
 */

#include "SoftwareSerial.h"


void SoftwareSerial::begin(int, int, bool, bool, bool)
{

}

size_t SoftwareSerial::write(char *message, int len)
{
	return len;
}

size_t SoftwareSerial::write(uint8_t c)
{
	return 1;
}

void SoftwareSerial::enableTx(bool Enable)
{

}

void SoftwareSerial::enableIntTx(bool Enable)
{

}

void SoftwareSerial::print(std::string Line)
{

}

void SoftwareSerial::println(std::string Line)
{

}

void SoftwareSerial::flush(void)
{

}

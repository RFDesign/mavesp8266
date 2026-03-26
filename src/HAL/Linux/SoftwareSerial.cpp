/*
 * SoftwareSerial.cpp
 *
 *  Created on: 18 Dec 2025
 *      Author: snh
 */

#include "SoftwareSerial.h"

SoftwareSerial::SoftwareSerial()
	: _Serial(2)
{
}

void SoftwareSerial::begin(int BaudRate, int PacketType, bool Rx, bool Tx, bool Invert)
{
	_Serial.begin(BaudRate);
}

size_t SoftwareSerial::write(char *message, int len)
{
	if (_TxEnabled)
	{
		return _Serial.write((uint8_t *)message, len);
	}
	else
	{
		return len;
	}
}

void SoftwareSerial::write(uint8_t x)
{
	if (_TxEnabled)
	{
		_Serial.write((char)x);
	}
}

void SoftwareSerial::enableTx(bool Enable)
{
	_TxEnabled = Enable;
}

void SoftwareSerial::enableIntTx(bool Enable)
{
	_TxEnabled = Enable;
}

void SoftwareSerial::print(std::string Line)
{
	write((char *)Line.c_str(), Line.size());
}

void SoftwareSerial::println(std::string Line)
{
	print(Line);
	print("\n");
}

void SoftwareSerial::flush(void)
{
	_Serial.flush();
}


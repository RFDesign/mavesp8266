/*
 * WiFi.cpp
 *
 *  Created on: 7 Jan 2026
 *      Author: snh
 */

#include "WiFi.h"
#include "submodules/RFDProxy/RFDLib/Net/Net.hpp"

void TWiFi::begin(char *a, char *b)
{

}

void TWiFi::disconnect(bool b)
{

}

void TWiFi::macAddress(byte *p)
{
	uint64_t mac = RFDLib::Net::GetLocalMACAddress();
	uint8_t *pMAC = (uint8_t *)&mac;

	for (int n = 0; n < 6; n++)
	{
		p[n] = pMAC[5 - n];
	}
}

void TWiFi::softAPmacAddress(byte *p)
{
	macAddress(p);
}

void TWiFi::mode(int x)
{

}

void TWiFi::config(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e)
{

}

int TWiFi::status(void)
{
	return 0;
}

IPAddress TWiFi::localIP(void)
{
	return IPAddress(RFDLib::Net::GetLocalIPAddress());
}

void TWiFi::setAutoReconnect(bool)
{

}

void TWiFi::encryptionType(int a)
{

}

void TWiFi::softAP(char *a, char *b, uint32_t c)
{

}

IPAddress TWiFi::softAPIP(void)
{
	return IPAddress(RFDLib::Net::GetLocalIPAddress());
}

void TWiFi::setOutputPower(float)
{

}

TWiFi WiFi;

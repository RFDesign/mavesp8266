/*
 * WiFiUdp.cpp
 *
 *  Created on: 16 Oct 2025
 *      Author: snh
 */

#include "WiFiUdp.h"

void WiFiUDP::begin(uint16_t Port)
{

}

void WiFiUDP::beginPacket(IPAddress &ip, uint16_t Port)
{

}

size_t WiFiUDP::write(unsigned char *message, int len)
{
	return len;
}

size_t WiFiUDP::write(char *message, int len)
{
	return len;
}

void WiFiUDP::endPacket(void)
{

}

int WiFiUDP::parsePacket(void)
{
	return 0;
}

int WiFiUDP::read(void)
{
	return 0;
}

IPAddress WiFiUDP::remoteIP(void)
{
	return IPAddress();
}

void WiFiUDP::stopAll(void)
{

}

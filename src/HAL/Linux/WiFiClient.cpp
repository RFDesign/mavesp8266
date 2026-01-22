/*
 * WiFiClient.cpp
 *
 *  Created on: 8 Jan 2026
 *      Author: snh
 */

#include "WiFiClient.h"

WiFiClient::WiFiClient()
{

}

WiFiClient::WiFiClient(const WiFiClient &ToCopy)
{

}

bool WiFiClient::connected(void)
{
	return false;
}

size_t WiFiClient::available(void)
{
	return 0;
}

size_t WiFiClient::write(const char *buf, int n)
{
	return n;
}

size_t WiFiClient::write(const uint8_t *buf, int n)
{
	return n;
}

size_t WiFiClient::write(File &f)
{
	return f.size();
}

size_t WiFiClient::write_P(const void *v, size_t n)
{
	return n;
}

char WiFiClient::read(void)
{
	return 0;
}

size_t WiFiClient::readBytes(uint8_t *x, size_t n)
{
	return n;
}

String WiFiClient::readStringUntil(char cx)
{
	return String();
}

void WiFiClient::setTimeout(int t)
{

}

void WiFiClient::flush(void)
{

}

void WiFiClient::stop(void)
{

}

void WiFiClient::setNoDelay(bool b)
{

}

WiFiClient::operator bool() const
{
	return false;
}

uint8_t wifi_softap_get_station_num(void)
{
	return 0;
}

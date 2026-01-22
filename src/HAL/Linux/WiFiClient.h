/*
 * WiFiClient.h
 *
 *  Created on: 15 Oct 2025
 *      Author: snh
 */

#ifndef HAL_LINUX_WIFICLIENT_H_
#define HAL_LINUX_WIFICLIENT_H_

#include "common.h"
#include "WiFi.h"
#include "FS.h"

class WiFiClient
{
public:
	WiFiClient();
	WiFiClient(const WiFiClient &ToCopy);
	bool connected(void);
	size_t available(void);
	size_t write(const char *buf, int);
	size_t write(const uint8_t *buf, int);
	size_t write(File &f);
	size_t write_P(const void *, size_t);
	char read(void);
	size_t readBytes(uint8_t *, size_t);
	String readStringUntil(char);
	void setTimeout(int);
	void flush(void);
	void stop(void);
	void setNoDelay(bool b);

	/*WiFiClient operator= (WiFiClient &Other)
	{
		return WiFiClient(Other);
	}*/

	void operator= (const WiFiClient &Other)
	{

	}

	operator bool() const;
};

uint8_t wifi_softap_get_station_num(void);


#endif /* HAL_LINUX_WIFICLIENT_H_ */

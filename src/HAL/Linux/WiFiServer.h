/*
 * WiFiServer.h
 *
 *  Created on: 16 Oct 2025
 *      Author: snh
 */

#ifndef HAL_LINUX_WIFISERVER_H_
#define HAL_LINUX_WIFISERVER_H_

#include "WiFi.h"
#include "WiFiClient.h"

class WiFiServer
{
public:
	typedef WiFiClient ClientType;
	WiFiServer(int);
	WiFiClient& available(void);
	void begin(void);
	void close(void);
};


#endif /* HAL_LINUX_WIFISERVER_H_ */

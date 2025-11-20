/*
 * WiFiUdp.h
 *
 *  Created on: 15 Oct 2025
 *      Author: snh
 */

#ifndef HAL_LINUX_WIFIUDP_H_
#define HAL_LINUX_WIFIUDP_H_

#include "common.h"

#include "IPAddress.h"

class WiFiUDP
{
public:
	void begin(uint16_t Port);
	void beginPacket(IPAddress &ip, uint16_t Port);
	size_t write(unsigned char *message, int len);
	size_t write(char *message, int len);
	void endPacket(void);
	int parsePacket(void);
	int read(void);
	IPAddress remoteIP(void);
	static void stopAll(void);
};

#endif /* HAL_LINUX_WIFIUDP_H_ */

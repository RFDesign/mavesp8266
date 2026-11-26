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
	WiFiUDP();
	~WiFiUDP();
	void begin(uint16_t Port);
	void beginPacket(IPAddress &ip, uint16_t Port);
	size_t write(unsigned char *message, int len);
	size_t write(char *message, int len);
	void endPacket(void);
	int parsePacket(void);
	int read(void);
	IPAddress remoteIP(void);
	static void stopAll(void);
private:
	static const int BUFFER_SIZE = 2048;

	void Close(void);

	int _FD = -1;
	uint32_t _TxIP = 0;
	uint16_t _TxPort = 0;
	uint32_t _RxRemoteIP = 0;
	unsigned char _TxBuffer[BUFFER_SIZE];
	unsigned char _RxBuffer[BUFFER_SIZE];
	int _TxLength = 0;
	int _RxLength = 0;
	int _RxOffset = 0;
};

#endif /* HAL_LINUX_WIFIUDP_H_ */

/*
 * WiFi.h
 *
 *  Created on: 12 Nov 2025
 *      Author: snh
 */

#ifndef SRC_HAL_LINUX_WIFI_H_
#define SRC_HAL_LINUX_WIFI_H_

#include "common.h"
#include "IPAddress.h"

#define WIFI_OFF 0
#define WIFI_STA 1
#define WIFI_AP 2
#define WL_CONNECTED 1
#define AUTH_WPA2_PSK 0

class TWiFi
{
public:
	void begin(char *, char *);
	void disconnect(bool);
	void macAddress(byte *p);
	void softAPmacAddress(byte *p);
	void mode(int);
	void config(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
	int status(void);
	IPAddress localIP(void);
	void setAutoReconnect(bool);
	void encryptionType(int);
	void softAP(char *, char *, uint32_t);
	IPAddress softAPIP(void);
	void setOutputPower(float);
};

String half_mac2String(byte *p);

extern TWiFi &WiFi;



#endif /* SRC_HAL_LINUX_WIFI_H_ */

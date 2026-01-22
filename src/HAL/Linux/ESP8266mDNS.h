/*
 * ESP8266mDNS.h
 *
 *  Created on: 16 Oct 2025
 *      Author: snh
 */

#ifndef HAL_LINUX_ESP8266MDNS_H_
#define HAL_LINUX_ESP8266MDNS_H_

#include <string>

class TMDNS
{
public:
	bool begin(char *);
	void update(void);
	void addService(std::string, std::string, int);
};

extern TMDNS MDNS;


#endif /* HAL_LINUX_ESP8266MDNS_H_ */

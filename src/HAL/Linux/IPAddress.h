/*
 * IPAddress.h
 *
 *  Created on: 30 Oct 2025
 *      Author: snh
 */

#ifndef SRC_HAL_LINUX_IPADDRESS_H_
#define SRC_HAL_LINUX_IPADDRESS_H_

#include "stdint.h"
#include <string>

//typedef uint32_t IPAddress;

class IPAddress
{
public:
	IPAddress();
	IPAddress(const IPAddress&);
	IPAddress(uint32_t);

	std::string toString(void);
	void fromString(const char *);

	uint8_t& operator[](int Index)
	{
		return _Octets[Index];
	}

	IPAddress operator= (uint32_t x)
	{
		return IPAddress(x);
	}

	operator uint32_t() const;
private:
	uint8_t _Octets[4];
};


#endif /* SRC_HAL_LINUX_IPADDRESS_H_ */

/*
 * IPAddress.cpp
 *
 *  Created on: 18 Dec 2025
 *      Author: snh
 */

#include "IPAddress.h"
#include "submodules/RFDProxy/RFDLib/Text/Text.hpp"
#include <string.h>


IPAddress::IPAddress()
{
	for (int n = 0; n < 4; n++)
	{
		_Octets[n] = 0;
	}
}

IPAddress::IPAddress(const IPAddress& Other)
{
	uint32_t x = Other.operator unsigned int();
	memcpy(_Octets, &x, 4);
}

IPAddress::IPAddress(uint32_t Other)
{
	memcpy(_Octets, &Other, 4);
}

std::string IPAddress::toString(void)
{
	std::string Result;

	for (int n = 3; n >= 0; n--)
	{
		if (Result.size() != 0)
		{
			Result += ".";
		}

		Result += std::to_string((int)_Octets[n]);
	}

	return Result;
}

void IPAddress::fromString(const char *s)
{
	RFDLib::Text::ParseIPv4Address(s, _Octets);
}

/*uint8_t& IPAddress::operator[](int Index)
{
	return _Octets[Index];
}*/

/*IPAddress IPAddress::operator= (uint32_t x)
{
	return IPAddress(x);
}*/

IPAddress::operator uint32_t() const
{
	return *((uint32_t *)_Octets);
}

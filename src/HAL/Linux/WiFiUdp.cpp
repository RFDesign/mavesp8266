/*
 * WiFiUdp.cpp
 *
 *  Created on: 16 Oct 2025
 *      Author: snh
 */

#include "WiFiUdp.h"
#include <algorithm>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

static std::vector<WiFiUDP *> &GetWiFiUDPInstances(void)
{
	static std::vector<WiFiUDP *> Instances;

	return Instances;
}

WiFiUDP::WiFiUDP()
{
	GetWiFiUDPInstances().push_back(this);
}

WiFiUDP::~WiFiUDP()
{
	Close();

	std::vector<WiFiUDP *> &Instances = GetWiFiUDPInstances();
	Instances.erase(std::remove(Instances.begin(), Instances.end(), this), Instances.end());
}

void WiFiUDP::Close(void)
{
	if (_FD >= 0)
	{
		close(_FD);
		_FD = -1;
	}

	_TxLength = 0;
	_RxLength = 0;
	_RxOffset = 0;
}

void WiFiUDP::begin(uint16_t Port)
{
	Close();

	_FD = socket(AF_INET, SOCK_DGRAM, 0);
	if (_FD < 0)
	{
		return;
	}

	int Enabled = 1;
	setsockopt(_FD, SOL_SOCKET, SO_REUSEADDR, &Enabled, sizeof(Enabled));
	setsockopt(_FD, SOL_SOCKET, SO_BROADCAST, &Enabled, sizeof(Enabled));

	sockaddr_in Addr;
	memset(&Addr, 0, sizeof(Addr));
	Addr.sin_family = AF_INET;
	Addr.sin_addr.s_addr = INADDR_ANY;
	Addr.sin_port = htons(Port);

	if (bind(_FD, (sockaddr *)&Addr, sizeof(Addr)) != 0)
	{
		Close();
		return;
	}

	int Flags = fcntl(_FD, F_GETFL, 0);
	if (Flags >= 0)
	{
		fcntl(_FD, F_SETFL, Flags | O_NONBLOCK);
	}
}

void WiFiUDP::beginPacket(IPAddress &ip, uint16_t Port)
{
	IPAddress i;
	i.fromString("127.0.0.1");

	_TxIP = i;//ip;
	_TxPort = Port;
	_TxLength = 0;
}

size_t WiFiUDP::write(unsigned char *message, int len)
{
	if (message == nullptr || len <= 0)
	{
		return 0;
	}

	int Space = BUFFER_SIZE - _TxLength;
	int ToCopy = len < Space ? len : Space;

	if (ToCopy > 0)
	{
		memcpy(_TxBuffer + _TxLength, message, ToCopy);
		_TxLength += ToCopy;
	}

	return ToCopy;
}

size_t WiFiUDP::write(char *message, int len)
{
	return write((unsigned char *)message, len);
}

void WiFiUDP::endPacket(void)
{
	if (_FD < 0 || _TxLength <= 0)
	{
		return;
	}

	sockaddr_in Addr;
	memset(&Addr, 0, sizeof(Addr));
	Addr.sin_family = AF_INET;
	Addr.sin_addr.s_addr = htonl(_TxIP);
	Addr.sin_port = htons(_TxPort);

	ssize_t QTYSent = sendto(_FD, _TxBuffer, _TxLength, 0, (sockaddr *)&Addr, sizeof(Addr));
	if (QTYSent < _TxLength)
	{
		printf("Failed to send UDP packet.  Sent %d instead of %d\n", QTYSent, _TxLength);
	}
	_TxLength = 0;
}

int WiFiUDP::parsePacket(void)
{
	if (_RxOffset < _RxLength)
	{
		return _RxLength - _RxOffset;
	}

	_RxLength = 0;
	_RxOffset = 0;

	if (_FD < 0)
	{
		return 0;
	}

	sockaddr_in Addr;
	socklen_t AddrLen = sizeof(Addr);
	int Result = recvfrom(_FD, _RxBuffer, sizeof(_RxBuffer), 0, (sockaddr *)&Addr, &AddrLen);

	if (Result > 0)
	{
		_RxLength = Result;
		_RxRemoteIP = ntohl(Addr.sin_addr.s_addr);
	}
	else if (Result < 0 && errno != EAGAIN && errno != EWOULDBLOCK)
	{
		Close();
	}

	return _RxLength;
}

int WiFiUDP::read(void)
{
	if (_RxOffset < _RxLength)
	{
		return _RxBuffer[_RxOffset++];
	}

	return -1;
}

IPAddress WiFiUDP::remoteIP(void)
{
	return IPAddress(_RxRemoteIP);
}

void WiFiUDP::stopAll(void)
{
	for (WiFiUDP *pUDP : GetWiFiUDPInstances())
	{
		pUDP->Close();
	}
}

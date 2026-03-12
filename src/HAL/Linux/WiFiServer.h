/*
 * WiFiServer.h
 *
 *  Created on: 16 Oct 2025
 *      Author: snh
 */

#ifndef HAL_LINUX_WIFISERVER_H_
#define HAL_LINUX_WIFISERVER_H_

#include <queue>
#include "WiFi.h"
#include "WiFiClient.h"
#include "submodules/RFDProxy/interfaces/TCPCommon.hpp"
#include "WiFiServerClientConnection.hpp"

/**
 * A TCP server.  Used by WiFiServer.
 */
class TTCPServer : public rfdproxy::interfaces::TBaseTCPServer<TWiFiServerClientConnection>
{
public:
	TTCPServer(uint16_t PortNumber);
	~TTCPServer();
	void AcceptNewClient(rfdproxy::interfaces::TTCPServerClientConnection &Conn) override;
	std::string GetTCPServerName(void) override;
	std::shared_ptr<TWiFiServerClientConnection> Accept(void);
private:
	std::queue<std::shared_ptr<TWiFiServerClientConnection>> _AcceptQueue;
};

/**
 * A TCP server.
 */
class WiFiServer
{
public:
	typedef WiFiClient ClientType;
	WiFiServer(int n);
	WiFiClient available(void);
	void begin(void);
	void close(void);
private:
	TTCPServer *_pTCPServer = nullptr;
	uint16_t _PortNumber;
};


#endif /* HAL_LINUX_WIFISERVER_H_ */

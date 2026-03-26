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
	TTCPServer(uint16_t PortNumber, RFDLib::Setting::TReadOnlySetting<bool> &ServerOpen);
	~TTCPServer();
	void AcceptNewClient(rfdproxy::interfaces::TTCPServerClientConnection &Conn) override;
	std::string GetTCPServerName(void) override;
	std::shared_ptr<TWiFiClientDisconnector> Accept(void);
private:
	std::queue<TWiFiServerClientConnection *> _AcceptQueue;
	RFDLib::Setting::TReadOnlySetting<bool> &_ServerOpen;
};

/**
 * A TCP server.
 */
class WiFiServer : public RFDLib::Setting::TReadOnlySetting<bool>
{
public:
	typedef WiFiClient ClientType;
	WiFiServer(int n);
	WiFiClient available(void);
	void begin(void);
	void close(void);
	bool GetValue(void) override;
private:
	TTCPServer *_pTCPServer = nullptr;
	uint16_t _PortNumber;
};


#endif /* HAL_LINUX_WIFISERVER_H_ */

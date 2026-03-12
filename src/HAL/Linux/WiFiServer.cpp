/*
 * WiFiServer.cpp
 *
 *  Created on: 8 Jan 2026
 *      Author: snh
 */

#include "WiFiServer.h"
#include "submodules/RFDProxy/RFDLib/Threading/Threading.hpp"

/**
 * Create a new TTCPServer, listening on PortNumber
 */
TTCPServer::TTCPServer(uint16_t PortNumber)
	: rfdproxy::interfaces::TBaseTCPServer<TWiFiServerClientConnection>(PortNumber == 80 ? 8080 : PortNumber, 10000, false)
{
}

/**
 * TTCPServer destructor.
 */
TTCPServer::~TTCPServer()
{
	Close();
}

/**
 * Accept a new client connection.
 */
void TTCPServer::AcceptNewClient(rfdproxy::interfaces::TTCPServerClientConnection &Conn)
{
	std::shared_ptr<TWiFiServerClientConnection> pWSCC(new TWiFiServerClientConnection(Conn));

	AddClient(*pWSCC);
	MUTEX_LOCK(_Mutex);
	_AcceptQueue.push(pWSCC);

	printf("Got new connection on %s\n", GetTCPServerName().c_str());
}

/**
 * @return the TCP server's name.
 */
std::string TTCPServer::GetTCPServerName(void)
{
	return "WiFiServer on port " + std::to_string(GetPortNumber());
}

/**
 * Accept the next client in the queue
 *
 * @return the next client connection, or nullptr if no next connection.
 */
std::shared_ptr<TWiFiServerClientConnection> TTCPServer::Accept(void)
{
	MUTEX_LOCK(_Mutex);
	if (_AcceptQueue.size() == 0)
	{
		return nullptr;
	}
	else
	{
		std::shared_ptr<TWiFiServerClientConnection> pResult(_AcceptQueue.front());
		_AcceptQueue.pop();
		printf("TTCPServer::Accept returned a new connection\n");
		return pResult;
	}
}

/**
 * Create a new WiFiServer, listening on PortNumber
 */
WiFiServer::WiFiServer(int PortNumber)
	: _PortNumber(PortNumber)
{
}

/**
 * Returns the next client connection.
 */
WiFiClient WiFiServer::available(void)
{
	if (_pTCPServer == nullptr)
	{
		return WiFiClient();
	}
	else
	{
		return WiFiClient(_pTCPServer->Accept());
	}
}

/**
 * Start listening.
 */
void WiFiServer::begin(void)
{
	if (_pTCPServer == nullptr)
	{
		_pTCPServer = new TTCPServer(_PortNumber);
	}
}

/**
 * Close the server.
 */
void WiFiServer::close(void)
{
	if (_pTCPServer != nullptr)
	{
		delete _pTCPServer;
		_pTCPServer = nullptr;
	}
}

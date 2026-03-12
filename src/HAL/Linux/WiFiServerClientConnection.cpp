/*
 * WiFiServerClientConnection.cpp
 *
 *  Created on: 28 Jan 2026
 *      Author: snh
 */


#include "WiFiServerClientConnection.hpp"

TWiFiServerClientConnection::TWiFiServerClientConnection(rfdproxy::interfaces::TTCPServerClientConnection &Conn)
	: rfdproxy::interfaces::TBackgroundIO(Conn)
{
}

TWiFiServerClientConnection::~TWiFiServerClientConnection()
{
	Stop();
	WaitUntilTxComplete();
	GetConnection().WaitUntilTxComplete();
	GetConnection().Close();
}

rfdproxy::interfaces::TTCPServerClientConnection& TWiFiServerClientConnection::GetConnection(void)
{
	return (rfdproxy::interfaces::TTCPServerClientConnection&)GetWrappedByteStream();
}

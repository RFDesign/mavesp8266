/*
 * WiFiServerClientConnection.hpp
 *
 *  Created on: 28 Jan 2026
 *      Author: snh
 */

#ifndef SRC_HAL_LINUX_WIFISERVERCLIENTCONNECTION_HPP_
#define SRC_HAL_LINUX_WIFISERVERCLIENTCONNECTION_HPP_

#include "submodules/RFDProxy/interfaces/TCPCommon.hpp"
#include "submodules/RFDProxy/interfaces/BackgroundIO.hpp"

class TWiFiServerClientConnection : public rfdproxy::interfaces::TBackgroundIO
{
public:
	TWiFiServerClientConnection(rfdproxy::interfaces::TTCPServerClientConnection &Conn);
	~TWiFiServerClientConnection();
	rfdproxy::interfaces::TTCPServerClientConnection& GetConnection(void);
private:
};

/*class TWiFiClientDisconnector
{
public:
	TWiFiClientDisconnector(TWiFiServerClientConnection &Conn);
	~TWiFiClientDisconnector();
	TWiFiServerClientConnection &GetConn(void);
private:
	TWiFiServerClientConnection &_Conn;
};*/





#endif /* SRC_HAL_LINUX_WIFISERVERCLIENTCONNECTION_HPP_ */

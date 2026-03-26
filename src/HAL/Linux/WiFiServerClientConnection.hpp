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
	void Close(void);
	rfdproxy::interfaces::TTCPServerClientConnection& GetConnection(void);
	bool GetIsTxEmpty(void) override;
	void DoClose(void);
	static void *ThreadStartTarget(void *p);
private:
	typedef enum
	{
		RUNNING,
		CLOSING,
		CLOSED
	} TState;
	TState _State = RUNNING;
	pthread_t _ClosingThread;
	int _Number;
};

class TWiFiClientDisconnector
{
public:
	TWiFiClientDisconnector(TWiFiServerClientConnection &Conn, RFDLib::Setting::TReadOnlySetting<bool> &ServerOpen);
	~TWiFiClientDisconnector();
	TWiFiServerClientConnection &GetConn(void);
	int GetNumber(void);
private:
	TWiFiServerClientConnection &_Conn;
	int _Number;
	RFDLib::Setting::TReadOnlySetting<bool> &_ServerOpen;
};





#endif /* SRC_HAL_LINUX_WIFISERVERCLIENTCONNECTION_HPP_ */

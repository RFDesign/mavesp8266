/*
 * WiFiServerClientConnection.cpp
 *
 *  Created on: 28 Jan 2026
 *      Author: snh
 */


#include "WiFiServerClientConnection.hpp"

static int gNumber = 0;

TWiFiServerClientConnection::TWiFiServerClientConnection(rfdproxy::interfaces::TTCPServerClientConnection &Conn)
	: rfdproxy::interfaces::TBackgroundIO(Conn),
	  _Number(gNumber++)
{
	//printf("TWiFiServerClientConnection %d constructor\n", _Number);
}

TWiFiServerClientConnection::~TWiFiServerClientConnection()
{
	//printf("TWiFiServerClientConnection %d destructor\n", _Number);

	switch (_State)
	{
		case CLOSING:
			GetConnection().Close();
			_State = CLOSED;
		case CLOSED:	//fall-through is intentional
			pthread_join(_ClosingThread, NULL);
			break;
	}

	//printf("TWiFiServerClientConnection %d destructor done\n", _Number);

}

void TWiFiServerClientConnection::Close(void)
{
	if (_State == RUNNING)
	{
		_State = CLOSING;
		pthread_create(&_ClosingThread, NULL, ThreadStartTarget, (void *)this);
	}
}

bool TWiFiServerClientConnection::GetIsTxEmpty(void)
{
	if (_State == CLOSED)
	{
		return true;
	}
	else
	{
		return rfdproxy::interfaces::TBackgroundIO::GetIsTxEmpty();
	}
}

rfdproxy::interfaces::TTCPServerClientConnection& TWiFiServerClientConnection::GetConnection(void)
{
	return (rfdproxy::interfaces::TTCPServerClientConnection&)GetWrappedByteStream();
}

void TWiFiServerClientConnection::DoClose(void)
{
	//printf("TWiFiServerClientConnection %d DoClose Stop\n", _Number);
	Stop();
	//printf("TWiFiServerClientConnection %d DoClose WaitUntilTxComplete\n", _Number);
	WaitUntilTxComplete();
	//printf("TWiFiServerClientConnection %d DoClose GetConnection().WaitUntilTxComplete()\n", _Number);
	GetConnection().WaitUntilTxComplete();
	//printf("TWiFiServerClientConnection %d DoClose GetConnection().Close()\n", _Number);
	GetConnection().Close();
	//printf("TWiFiServerClientConnection %d DoClose Done\n", _Number);
	_State = CLOSED;
}

void *TWiFiServerClientConnection::ThreadStartTarget(void *p)
{
	((TWiFiServerClientConnection *)p)->DoClose();
	return nullptr;  //Suppress compiler warnings.
}

TWiFiClientDisconnector::TWiFiClientDisconnector(TWiFiServerClientConnection &Conn,
		RFDLib::Setting::TReadOnlySetting<bool> &ServerOpen)
	: _Conn(Conn),
	  _Number(gNumber++),
	  _ServerOpen(ServerOpen)
{
	//printf("TWiFiClientDisconnector number %d constructor\n", _Number);
}

TWiFiClientDisconnector::~TWiFiClientDisconnector()
{
	//printf("TWiFiClientDisconnector number %d destructor\n", _Number);
	if (_ServerOpen.GetValue())
	{
		_Conn.Close();
	}
}

TWiFiServerClientConnection &TWiFiClientDisconnector::GetConn(void)
{
	return _Conn;
}

int TWiFiClientDisconnector::GetNumber(void)
{
	return _Number;
}


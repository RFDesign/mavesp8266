/*
 * WiFiClient.cpp
 *
 *  Created on: 8 Jan 2026
 *      Author: snh
 */

#include "WiFiClient.h"

WiFiClient::WiFiClient()
{
}

/**
 * Create a new wifi client
 * @param pConn - Can be nullptr if no actual connection.
 */
WiFiClient::WiFiClient(std::shared_ptr<TWiFiServerClientConnection> pConn)
{
	_pConnection = pConn;
}

/**
 * Clone a wifi client
 */
WiFiClient::WiFiClient(const WiFiClient &ToCopy)
{
	_pConnection = ToCopy._pConnection;
}

/**
 * @return whether connected.
 */
bool WiFiClient::connected(void)
{
	return _pConnection != nullptr && _pConnection->GetConnection().IsConnected();
}

size_t WiFiClient::available(void)
{
	return _pConnection == nullptr ? 0 : _pConnection->GetRxAvailable();
}

/**
 * Write the given buffer to the client
 *
 * @param buf - Must not be nullptr
 * @param n - The size of the buffer
 * @return QTY of bytes written.
 */
size_t WiFiClient::write(const char *buf, int n)
{
	if (_pConnection == nullptr)
	{
		return 0;
	}
	else
	{
		/*for (int i = 0; i < n; i++)
		{
			printf(">%c", buf[i]);
		}*/

		return _pConnection->Write((char *)buf, n);
	}
}

/**
 * Write the given buffer to the client
 *
 * @param buf - Must not be nullptr
 * @param n - The size of the buffer
 * @return QTY of bytes written.
 */
size_t WiFiClient::write(const uint8_t *buf, int n)
{
	return write((const char *)buf, n);
}

/**
 * Write the given file to the client
 *
 * @param f
 * @return QTY of bytes written.
 */
size_t WiFiClient::write(File &f)
{
	if (_pConnection == nullptr)
	{
		return 0;
	}
	else
	{
		RFDLib::Buffer::TArbLengthAppendableBuffer<1024> Buffer;
		size_t Result = 0;

		int Temp;

		while ((Temp = f.read()) != EOF)
		{
			if (!Buffer.AppendByte(Temp))
			{
				size_t Written = write((const char *)Buffer.Raw(), Buffer.GetQTYBytes());
				Result += Written;
				if (Written != Buffer.GetQTYBytes())
				{
					return Result;
				}
				Buffer.Clear();
				// Buffer was full; append the byte that didn't fit.
				Buffer.AppendByte(Temp);
			}
		}

		if (Buffer.GetQTYBytes() != 0)
		{
			Result += write((const char *)Buffer.Raw(), Buffer.GetQTYBytes());
		}

		return Result;
	}
}

/**
 * Write the given buffer to the client
 *
 * @param v - Must not be nullptr
 * @param n - The size of the buffer
 * @return QTY of bytes written.
 */
size_t WiFiClient::write_P(const void *v, size_t n)
{
	return write((const char *)v, n);
}

/**
 * Read a byte from the client
 *
 * @return the byte read, or EOF if no bytes to read.
 */
int WiFiClient::read(void)
{
	if (_pConnection == nullptr)
	{
		return EOF;
	}
	else
	{
		int Result = _pConnection->ReadByte();
		/*if (Result != EOF)
		{
			printf("<%c", Result);
		}*/
		return Result;
	}
}

/**
 * Read into the given buffer
 *
 * @param x - Must not be nullptr
 * @param n - The size of the buffer
 * @return QTY of bytes read
 */
size_t WiFiClient::readBytes(uint8_t *x, size_t n)
{
	if (_pConnection == nullptr)
	{
		return 0;
	}
	else
	{
		int Result = _pConnection->Read((char *)x, n);
		/*if (Result != 0)
		{
			printf("WiFiClient::readBytes got %d bytes\n", Result);
		}*/
		return Result;
	}
}

/**
 * Read from the client until the given character is encountered, or time out.
 *
 * @param cx
 * @return the resulting string read
 */
String WiFiClient::readStringUntil(char cx)
{
	RFDLib::Buffer::TArbLengthAppendableBuffer<10 * 1024> Buffer;
	uint64_t LastRxByte = rfdproxy::time::GetMonotonicTimeInMilliseconds();

	while (true)
	{
		int Temp;

		Temp = read();

		if (Temp == EOF)
		{
			if (rfdproxy::time::GetMonotonicTimeInMilliseconds() - LastRxByte > 3000)
			{
				break;
			}
			else
			{
				rfdproxy::time::MSleep(2);
			}
		}
		else
		{
			LastRxByte = rfdproxy::time::GetMonotonicTimeInMilliseconds();
			if (Temp == cx)
			{
				break;
			}
			else
			{
				Buffer.AppendByte(Temp);
			}
		}
	}

	Buffer.AppendByte(0);

	return String(Buffer.Raw());
}

void WiFiClient::setTimeout(int t)
{

}

void WiFiClient::flush(void)
{

}

/**
 * Disconnect.
 */
void WiFiClient::stop(void)
{
	_pConnection.reset();
}

void WiFiClient::setNoDelay(bool b)
{

}

TWiFiServerClientConnection* WiFiClient::GetConnection(void) const
{
	return _pConnection.get();
}

WiFiClient::operator bool() const
{
	return _pConnection != nullptr;
}

uint8_t wifi_softap_get_station_num(void)
{
	return 0;
}

/*
 * common.cpp
 *
 *  Created on: 26 Nov 2025
 *      Author: snh
 */

#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include "common.h"

#include "submodules/RFDProxy/time/monotonic.hpp"
#include "submodules/RFDProxy/interfaces/TCPCommon.hpp"

void setup(void);
void loop(void);
void CtrlCHandler(int sig_no);

char *gProgName = nullptr;
char **gArgV = nullptr;

uint32_t _SPIFFS_start = 0;
uint32_t _SPIFFS_end = 1024*1024;
static bool gRun = true;

/**
 * HAL main function
 */
int main(int argc, const char *argv[])
{
	gProgName = (char *)argv[0];
	gArgV = (char **)argv;

	rfdproxy::time::Init();

	signal(SIGINT, CtrlCHandler);

	GetSerial().begin(57600);

	setup();

	while (gRun)
	{
		loop();
	}

	rfdproxy::interfaces::DeInitIfNecessary();

	return 0;
}

void CtrlCHandler(int sig_no)
{
    printf("CTRL-C pressed\n");
    gRun = false;
}

// -----------------------------------------------------------------------------
// Free functions
// -----------------------------------------------------------------------------

uint64_t millis(void)
{
	return rfdproxy::time::GetMonotonicTimeInMilliseconds();
}

void delay(int Milliseconds)
{
	rfdproxy::time::MSleep(Milliseconds);
}

/**
 * Get the serial port with the given port number and baud rate.  Opens the port if not already open.
 *
 * @param PortNumber
 * @param BaudRate
 * @return the serial port.  Never nullptr.
 */
std::shared_ptr<rfdproxy::interfaces::TSerialPort> TSerial::GetPort(int PortNumber, int BaudRate)
{
	static std::map<int, std::shared_ptr<rfdproxy::interfaces::TSerialPort>> _Ports;

	auto i = _Ports.find(PortNumber);

	if (i == _Ports.end())
	{
		std::string File = "/dev/ttyUSB" + std::to_string(PortNumber - 1);
		std::shared_ptr<rfdproxy::interfaces::TSerialPort> Result(new rfdproxy::interfaces::TSerialPort(File, BaudRate, false));
		_Ports[PortNumber] = Result;
		return Result;
	}
	else
	{
		i->second->SetAttributes(BaudRate, false);
		return i->second;
	}
}

// -----------------------------------------------------------------------------
// TSerial
// -----------------------------------------------------------------------------

void PrintChar(char c)
{
	if (isprint(c))
	{
		putchar(c);
	}
	else
	{
		printf("<0x%02X>", (uint8_t)c);
	}
}

/**
 * TSerial constructor
 */
TSerial::TSerial()
{
}

/**
 * Create a new TSerial with the given port number.
 */
TSerial::TSerial(int PortNumber)
	: _PortNumber(PortNumber)
{
}

void TSerial::begin(int BaudRate)
{
	_SP = GetPort(_PortNumber, BaudRate);
}

/**
 * Close this TSerial
 */
void TSerial::end(void)
{
	_SP = nullptr;
}

/**
 * Set the read timeout in milliseconds.
 */
void TSerial::setTimeout(int x)
{
	_ReadTimeout = x;
}

void TSerial::setRxBufferSize(int)
{
	// TODO: Implement RX buffer resize
}

/**
 * Read a byte from the serial port.
 *
 * @return the read byte, or EOF if nothing to read.
 */
int TSerial::read(void)
{
	char Result;

	if (readBytes(&Result, 1) == 0)
	{
		return EOF;
	}
	else
	{
		return Result;
	}
}

/**
 * Read from the TSerial into the given buffer.
 *
 * @param Dest - The buffer.  Must not be nullptr.
 * @param Length - The length of the buffer in bytes.
 * @return the QTY of bytes read.
 */
size_t TSerial::readBytes(char *Dest, int Length)
{
	if (_SP == nullptr)
	{
		return 0;
	}
	else
	{
		int Result = readBytesWithTimeout(Dest, Length);
		/*for (int n = 0; n < Result; n++)
		{
			putchar('<');
			PrintChar(Dest[n]);
		}*/

		return Result;
	}
}

/*
 * @return the number of bytes available to read.  Never negative.
 */
int TSerial::available(void)
{
	if (_SP == nullptr)
	{
		return 0;
	}
	else
	{
		int Result = _SP->GetAvailableToRead();
		if (Result < 0)
		{
			return 0;
		}
		else
		{
			return Result;
		}
	}
}

size_t TSerial::availableForWrite(void)
{
	if (_SP == nullptr)
	{
		return 0;
	}
	else
	{
		return _SP->GetAvailableToWrite();
	}
}

/**
 * Write to the serial port.
 *
 * @param message - The buffer.  Must not be nullptr.
 * @param len - The buffer length.
 * @return the QTY of bytes written.
 */
size_t TSerial::write(uint8_t *message, int len)
{
	if (_SP == nullptr)
	{
		return 0;
	}
	else
	{
		/*for (int n = 0; n < len; n++)
		{
			putchar('>');
			PrintChar(message[n]);
		}*/

		return _SP->Write((char *)message, len);
	}
}

/**
 * Write the given string to the serial port.
 *
 * @param str - The string to write.
 * @return the QTY of bytes written.
 */
size_t TSerial::write(const char *str)
{
	int len = strlen(str);
	//printf("\"%s\" has %d characters\n", str, len);

	return write((uint8_t *)str, len);
}

/**
 * Write the given character to the serial port.
 */
void TSerial::write(char x)
{
	write((uint8_t *)&x, 1);
}

/**
 * Flush write buffer to serial port before returning.
 */
void TSerial::flush(void)
{
	if (_SP == nullptr)
	{
		return;
	}
	else
	{
		while (_SP->GetTxBufferedBytes() != 0)
		{
			rfdproxy::time::MSleep(2);
		}
	}
}

void TSerial::setDebugOutput(bool b)
{

}

/**
 * Read from the TSerial into the given buffer, until buffer full or timeout expored.
 *
 * @param Dest - The buffer.  Must not be nullptr.
 * @param Length - The length of the buffer in bytes.
 * @return the QTY of bytes read.
 */
size_t TSerial::readBytesWithTimeout(char *Dest, int Length)
{
	if (_SP == nullptr)
	{
		return 0;
	}
	else
	{
		if (_ReadTimeout == 0)
		{
			return _SP->Read(Dest, Length);
		}
		else
		{
			uint64_t Start = rfdproxy::time::GetMonotonicTimeInMilliseconds();
			int Result = 0;

			while (Result < Length)
			{
				Result += _SP->Read(Dest + Result, Length - Result);
				if (rfdproxy::time::GetMonotonicTimeInMilliseconds() - Start > _ReadTimeout)
				{
					break;
				}
				if (Result < Length)
				{
					rfdproxy::time::MSleep(2);
				}
			}

			return Result;
		}
	}
}

// -----------------------------------------------------------------------------
// TESP
// -----------------------------------------------------------------------------

size_t TESP::getFreeSketchSpace(void)
{
	return 4 * 1024 * 1024;
}

void TESP::reset(void)
{
	execvp(gProgName, gArgV);
}

void TESP::restart(void)
{
	reset();
}

size_t TESP::getFreeHeap(void)
{
	//txmod only uses this to display for info purposes.  So just make something up.
	return 16 * 1024 * 1024;
}

size_t TESP::getFlashChipRealSize(void)
{
	//This just needs to be the same as the value returned by getFlashChipSize
	return 2 * 1024 * 1024;
}

size_t TESP::getFlashChipSize(void)
{
	//This just needs to be the same as the value returned by getFlashChipSize
	return getFlashChipRealSize();
}

// -----------------------------------------------------------------------------
// TUpdate
// -----------------------------------------------------------------------------

bool TUpdate::hasError(void)
{
	return false;
}

bool TUpdate::begin(size_t x)
{
	// TODO: Implement update begin
	return true;
}

bool TUpdate::begin(size_t, int Index)
{
	return true;
}

bool TUpdate::write(uint8_t *data, size_t len)
{
	// TODO: Implement update data write
	return true;
}

bool TUpdate::end(bool evenIfRemaining)
{
	// TODO: Implement update end
	return true;
}

bool TUpdate::end(void)
{
	return true;
}

void TUpdate::printError(std::string &s)
{

}

void TUpdate::printError(TSerial &s)
{

}

// -----------------------------------------------------------------------------
// Global singletons and helpers
// -----------------------------------------------------------------------------

static TUpdate g_updateInstance;
static TESP    g_espInstance;
static TSerial g_serialInstance(1);

TUpdate &Update = g_updateInstance;
TESP    &ESP    = g_espInstance;

TSerial &GetSerial(void)
{
	return g_serialInstance;
}

bool wifi_softap_dhcps_start(void)
{
	// TODO: Implement soft-AP DHCP server start
	return true;
}

int min(int a, int b)
{
	return (a < b) ? a : b;
}

/*
 * common.cpp
 *
 *  Created on: 26 Nov 2025
 *      Author: snh
 */

#include <stdlib.h>
#include <unistd.h>
#include "common.h"

#include "submodules/RFDProxy/time/monotonic.hpp"

void setup(void);
void loop(void);

char *gProgName = nullptr;
char **gArgV = nullptr;

uint32_t _SPIFFS_start = 0;
uint32_t _SPIFFS_end = 1024*1024;


int main(int argc, const char *argv[])
{
	gProgName = (char *)argv[0];
	gArgV = (char **)argv;

	setup();

	while (true)
	{
		loop();
	}

	return 0;
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

// -----------------------------------------------------------------------------
// TSerial
// -----------------------------------------------------------------------------

void TSerial::begin(int PortNumber)
{
	std::string File = "/dev/ttyUSB" + std::to_string(PortNumber);

	_SP.reset(new rfdproxy::interfaces::TSerialPort(File, 57600, true));
}

void TSerial::end(void)
{
	_SP = nullptr;
}

void TSerial::setRxBufferSize(int)
{
	// TODO: Implement RX buffer resize
}

int TSerial::read(void)
{
	if (_SP == nullptr)
	{
		return -1;
	}
	else
	{
		return _SP->ReadByte();
	}
}

int TSerial::available(void)
{
	if (_SP == nullptr)
	{
		return 0;
	}
	else
	{
		return _SP->GetAvailableToRead();
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

size_t TSerial::write(uint8_t *message, int len)
{
	if (_SP == nullptr)
	{
		return 0;
	}
	else
	{
		return _SP->Write((char *)message, len);
	}
}

size_t TSerial::write(const char *str)
{
	return write((uint8_t *)str, strlen(str));
}

void TSerial::flush(void)
{
	if (_SP == nullptr)
	{
		return;
	}
	else
	{
		while (_SP->GetAvailableToWrite() != 0)
		{
			rfdproxy::time::MSleep(2);
		}
	}
}

void TSerial::setDebugOutput(bool b)
{

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
static TSerial g_serialInstance;

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



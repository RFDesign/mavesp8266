/*
 * EEPROM.cpp
 *
 *  Created on: 4 Dec 2025
 *      Author: snh
 */

#include "EEPROM.h"


void TEEPROM::begin(uint16_t Port)
{
	for (int n = 0; n < SIZE; n++)
	{
		_EEPROM[n] = 0xFF;
	}

	FILE *fp = fopen(FILE_NAME.c_str(), "r");

	if (fp != nullptr)
	{
		const int CHUNK_SIZE = 1024;

		fread(_EEPROM, CHUNK_SIZE, SIZE / CHUNK_SIZE, fp);
		fclose(fp);
	}
}

/*size_t TEEPROM::write(unsigned char *message, int len)
{

}*/

/*size_t TEEPROM::write(uint32_t Address, unsigned char *message)
{

}*/

size_t TEEPROM::write(uint32_t Address, unsigned char b)
{
	if (Address < SIZE)
	{
		_EEPROM[Address] = b;
		return 1;
	}
	else
	{
		return 0;
	}
}

/*void TEEPROM::endPacket(void)
{

}

int TEEPROM::parsePacket(void)
{

}*/

uint8_t TEEPROM::read(int Address)
{
	if (Address < SIZE)
	{
		return _EEPROM[Address];
	}
	else
	{
		return 0;
	}

}

/*uint8_t* TEEPROM::getDataPtr(int Address)
{

}*/

uint8_t* TEEPROM::getDataPtr(void)
{
	return _EEPROM;
}

/*size_t TEEPROM::getFreeSketchSpace(void)
{

}*/

void TEEPROM::put(uint32_t Address, uint32_t Value)
{
	memcpy(_EEPROM + Address, &Value, sizeof(Value));
}

void TEEPROM::commit(void)
{
	//Save to file.
	FILE *fp = fopen(FILE_NAME.c_str(), "w");

	if (fp != nullptr)
	{
		const int CHUNK_SIZE = 1024;

		fwrite(_EEPROM, CHUNK_SIZE, SIZE / CHUNK_SIZE, fp);
		fclose(fp);
	}

}

bool TEEPROM::get(uint32_t Address, uint32_t &x)
{
	memcpy(_EEPROM + Address, &x, sizeof(uint32_t));
	return true;
}

TEEPROM EEPROM;




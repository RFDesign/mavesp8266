/*
 * EEPROM.h
 *
 *  Created on: 15 Oct 2025
 *      Author: snh
 */

#ifndef HAL_LINUX_EEPROM_H_
#define HAL_LINUX_EEPROM_H_


#include "common.h"

class TEEPROM
{
public:
	void begin(uint16_t Port);
	size_t write(unsigned char *message, int len);
	size_t write(uint32_t, unsigned char *message);
	size_t write(uint32_t, unsigned char message);
	void endPacket(void);
	int parsePacket(void);
	uint8_t read(int Address);
	uint8_t* getDataPtr(int Address);
	uint8_t* getDataPtr(void);
	size_t getFreeSketchSpace(void);
	void put(uint32_t, uint32_t);
	void commit(void);
	bool get(uint32_t Address, uint32_t &x);
};

extern TEEPROM& EEPROM;


#endif /* HAL_LINUX_EEPROM_H_ */

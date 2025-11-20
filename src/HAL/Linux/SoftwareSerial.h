/*
 * SoftwareSerial.h
 *
 *  Created on: 30 Oct 2025
 *      Author: snh
 */

#ifndef SRC_HAL_LINUX_SOFTWARESERIAL_H_
#define SRC_HAL_LINUX_SOFTWARESERIAL_H_

#include "common.h"

#define SWSERIAL_8N1 0
#define D2 0

class SoftwareSerial
{
public:
	void begin(int, int, bool, bool, bool);
	size_t write(char *message, int len);
	size_t write(uint8_t);
	void enableTx(bool Enable);
	void enableIntTx(bool Enable);
	void print(std::string Line);
	void println(std::string Line);
	void flush(void);
};



#endif /* SRC_HAL_LINUX_SOFTWARESERIAL_H_ */

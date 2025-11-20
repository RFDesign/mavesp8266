/*
 * common.h
 *
 *  Created on: 16 Oct 2025
 *      Author: snh
 */

#ifndef HAL_LINUX_COMMON_H_
#define HAL_LINUX_COMMON_H_

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
//#include <SoftwareSerial.h>
//#include <StdioSerial.h>
#include <Stream.h>
//#include <Esp.h>
#include "Arduino.h"

#define PI (M_PI)
#define PSTR(x) (x)
#define FPSTR(x) (x)

typedef uint8_t byte;

uint64_t millis(void);
void delay(int);

class TSerial : public Stream
{
public:
	void begin(int);
	void end(void);
	void setRxBufferSize(int);
	int read(void);
	int available(void);
	size_t availableForWrite(void);
	size_t write(uint8_t *message, int len);
	size_t write(const char *);
	void flush(void);
};


class TESP
{
public:
	size_t getFreeSketchSpace(void);
	void reset(void);
	void restart(void);
	size_t getFreeHeap(void);
	size_t getFlashChipRealSize(void);
	size_t getFlashChipSize(void);
};

class TUpdate
{
public:
	bool hasError(void);
	bool begin(size_t);
	bool write(uint8_t *, size_t);
	bool end(bool);
};

extern TUpdate &Update;

extern TESP &ESP;

TSerial& GetSerial(void);

#define Serial (GetSerial())

bool wifi_softap_dhcps_start(void);
int min(int, int);

#endif /* HAL_LINUX_COMMON_H_ */

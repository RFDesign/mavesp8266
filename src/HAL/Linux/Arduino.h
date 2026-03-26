/*
 * Arduino.h
 *
 *  Created on: 30 Oct 2025
 *      Author: snh
 */

#ifndef SRC_HAL_LINUX_ARDUINO_H_
#define SRC_HAL_LINUX_ARDUINO_H_

#include <string>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory>
#include <map>
#include "submodules/RFDProxy/interfaces/Serial.hpp"

#define IRAM_ATTR
#define PROGMEM

#define U_FLASH 0
#define U_FS 1

#define INPUT_PULLUP 1
#define FALLING 0
#define DEBUGV printf

#define ENABLE_SOFTDEBUG

#define F(string_literal) (FPSTR(PSTR(string_literal)))

#define DEC 10
#define HEX 16

/**
 * Emulates Arduino's String class.  Extends std::string
 */
class String : public std::string
{
public:
	String();
	String(const char *);
	String(std::string);
	String(int NumberToConvertToString);
	String(int NumberToConvertToString, int Base);

	String substring(int, int) const;
	String substring(int n) const;
	int toInt(void) const;

	String& operator=(const char *x)
	{
		this->assign(x ? x : "");
		return *this;
	}

	String& operator=(std::string x)
	{
		this->assign(x);
		return *this;
	}

	String operator+(int y) const
	{
		return String(std::string(*this) + std::to_string(y));
	}

	String operator+(uint16_t y) const
	{
		return String(std::string(*this) + std::to_string(y));
	}

	String operator+(char y) const
	{
		return String(std::string(*this) + y);
	}

	String operator+(const char *y) const
	{
		return String(std::string(*this) + (y ? y : ""));
	}

	String operator+(const String &y) const
	{
		return String(std::string(*this) + std::string(y));
	}

	String& operator+=(int y)
	{
		this->append(std::to_string(y));
		return *this;
	}

	String& operator+=(long y)
	{
		this->append(std::to_string(y));
		return *this;
	}

	String& operator+=(unsigned int y)
	{
		this->append(std::to_string(y));
		return *this;
	}

	String& operator+=(unsigned long y)
	{
		this->append(std::to_string(y));
		return *this;
	}

	String& operator+=(uint16_t y)
	{
		this->append(std::to_string(y));
		return *this;
	}

	String& operator+=(char y)
	{
		this->push_back(y);
		return *this;
	}

	String& operator+=(const char *y)
	{
		this->append(y ? y : "");
		return *this;
	}

	String& operator+=(const String &y)
	{
		this->append(std::string(y));
		return *this;
	}

	/*bool operator==(const String& Other) const
	{
		return equals(Other);
	}*/

	char charAt(int Offet) const;
	int indexOf(const std::string &x) const;
	int indexOf(const char x, int n) const;
	int indexOf(const char x) const;
	void trim(void);

	bool startsWith(std::string s);
	bool endsWith(std::string s) const;
	int length(void) const;

	bool equals(const String&) const;
	bool equalsConstantTime(char *);
	bool equalsIgnoreCase(const char *);
	bool equalsIgnoreCase(const String);

	bool reserve(size_t);
	void replace(const char *, const char *);
	void replace(String, String);
private:
	//std::string _Inner;
};

extern String emptyString;

/**
 * Generic stream interface
 */
class Stream
{
public:
	virtual void write(char x) = 0;
	virtual void setTimeout(int x) = 0;
	virtual size_t readBytes(char *Dest, int Length) = 0;
};

/**
 * A wrapper of a serial port, where there can be multiple such concurrent wrappers
 * for one serial port.
 */
class TSerial : public Stream
{
public:
	TSerial();
	TSerial(int PortNumber);
	void begin(int);
	void end(void);
	void setTimeout(int x) override;
	void setRxBufferSize(int);
	int read(void);
	size_t readBytes(char *Dest, int Length) override;
	int available(void);
	size_t availableForWrite(void);
	size_t write(uint8_t *message, int len);
	size_t write(const char *);
	void write(char x) override;
	void flush(void);
	void setDebugOutput(bool b);
private:
	size_t readBytesWithTimeout(char *Dest, int Length);
	int _PortNumber = 1;
	int _ReadTimeout = 0;
	std::shared_ptr<rfdproxy::interfaces::TSerialPort> _SP;
	static std::shared_ptr<rfdproxy::interfaces::TSerialPort> GetPort(int PortNumber, int BaudRate);
};

class HardwareSerial : public TSerial
{
public:
	HardwareSerial(int PortNumber);
};

class MD5Builder
{
public:
	void begin(void);
	void add(String);
	void calculate(void);
	String toString(void);
};

typedef uint8_t uint8;
typedef void* PGM_VOID_P;
//typedef void* PGM_P;


#define OUTPUT 1
#define INPUT 0

#define LOW 0
#define HIGH 1

void pinMode(int, int);
void digitalWrite(int, int);
bool digitalRead(int);

void interrupts(void);
void noInterrupts(void);
void attachInterrupt(int, void (*)(void), int);

bool isDigit(char);

size_t ets_vsnprintf(char *, int, const char *, va_list &);

size_t base64_encode_expected_len(size_t);
size_t base64_encode_chars(char *, size_t, char *);
uint32_t GetRandom32(void);

void yield(void);

size_t strlen_P(const void *);
void memccpy_P(void *, void *, int, size_t);

void ArduinoPrint(const char *s);
void ArduinoPrint(String s);
void ArduinoPrintLn(const char *s);
void ArduinoPrintLn(String s);

#define RANDOM_REG32 GetRandom32()


#endif /* SRC_HAL_LINUX_ARDUINO_H_ */

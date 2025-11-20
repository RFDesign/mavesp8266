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

#define IRAM_ATTR
#define PROGMEM

#define INPUT_PULLUP 1
#define FALLING 0
#define DEBUGV printf

class String : public std::string
{
public:
	String();
	String(const char *);
	String(std::string);
	String(size_t n);

	String substring(int, int) const;
	String substring(int n) const;
	int toInt(void) const;

	String operator= (const char *x)
	{
		return String(x);
	}

	String operator= (std::string x)
	{
		return String(x);
	}

	String operator+(int y)
	{
		return String(*this + std::to_string(y));
	}

	char charAt(int Offet) const;
	int indexOf(const std::string &x) const;
	int indexOf(const char x, int n) const;
	int indexOf(const char x) const;
	void trim(void);

	bool startsWith(std::string s);
	bool endsWith(std::string s) const;
	int length(void) const;

	bool equals(String);
	bool equalsConstantTime(char *);
	bool equalsIgnoreCase(const char *);

	bool reserve(size_t);
	void replace(const char *, const char *);
	void replace(String, String);
};

extern String &emptyString;

class Stream
{
public:
	void write(char);
	void setTimeout(int);
	void readBytes(char *, int);
};

class HardwareSerial : public Stream
{
public:
	HardwareSerial(int);
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

#define RANDOM_REG32 GetRandom32()


#endif /* SRC_HAL_LINUX_ARDUINO_H_ */

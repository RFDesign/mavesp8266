/*
 * FS.h
 *
 *  Created on: 30 Oct 2025
 *      Author: snh
 */

#ifndef SRC_HAL_LINUX_FS_H_
#define SRC_HAL_LINUX_FS_H_

#include <stdio.h>
#include "Arduino.h"

typedef const char * PGM_P;

class File
{
public:
	File();
	File(const File &);
	void setTimeout(int);
	String readStringUntil(char);
	String readString(void);
	operator void*() const;
	void print(String);
	void println(String);
	void close(void);
	bool available(void);
	int size(void);
	void seek(int);
	int position(void);
	char read(void);
	void write(const uint8_t *, int);
	String name(void);
};

class Dir
{
public:
	Dir();
	Dir(const Dir &);
	bool next(void);
	File openFile(std::string s);
};

namespace fs
{
class FS
{
public:
	File open(String s, const char *);
	bool exists(const char* s);
	//bool exists(std::string &s);
	bool exists(std::__cxx11::basic_string<char> s);
};

}

typedef fs::FS FS;

class TSPIFFS : public fs::FS
{
public:
	bool begin(void);
	void remove(int Key);
	void remove(String Key);
	void format(void);
	Dir openDir(std::string d);
	bool rename(std::string From, std::string To);
};



extern TSPIFFS &SPIFFS;

uint32_t spi_flash_get_id(void);
int system_get_flash_size_map(void);

#endif /* SRC_HAL_LINUX_FS_H_ */

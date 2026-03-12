/*
 * FS.h
 *
 *  Created on: 30 Oct 2025
 *      Author: snh
 */

#ifndef SRC_HAL_LINUX_FS_H_
#define SRC_HAL_LINUX_FS_H_

#include <stdio.h>
#include <memory>
#include <dirent.h>
#include "Arduino.h"

typedef const char * PGM_P;

class BaseFile
{
public:
	virtual void setTimeout(int) = 0;
	String readStringUntil(char);
	String readString(void);
	//operator void*() const;
	virtual operator bool() const = 0;
	void print(String s);
	void println(String l);
	virtual void close(void) = 0;
	virtual bool available(void) = 0;
	virtual int size(void) = 0;
	virtual void seek(int) = 0;
	virtual int position(void) = 0;
	virtual int read(void) = 0;
	virtual void write(const uint8_t *, int) = 0;
	virtual String name(void) = 0;
};

class TLinuxFile : public BaseFile
{
public:
	TLinuxFile(std::string Name, std::string Path, std::string Mode);
	operator bool() const override;
	void setTimeout(int t) override;
	void close(void) override;
	bool available(void) override;
	int size(void) override;
	void seek(int Position) override;
	int position(void) override;
	int read(void) override;
	void write(const uint8_t *Buffer, int Length) override;
	String name(void) override;
private:
	String _Name;
	FILE *_fp;
};

class File : public BaseFile
{
public:
	File();
	File(const File &);
	File(std::shared_ptr<BaseFile> f);
	operator bool() const override;
	void setTimeout(int) override;
	void close(void) override;
	bool available(void) override;
	int size(void) override;
	void seek(int) override;
	int position(void) override;
	int read(void) override;
	void write(const uint8_t *, int) override;
	String name(void) override;
	std::shared_ptr<BaseFile> GetBaseFile(void) const;
private:
	std::shared_ptr<BaseFile> _pBaseFile;
};

class TBaseDir
{
public:
	virtual bool next(void) = 0;
	virtual File openFile(std::string s) = 0;
	virtual String fileName(void) = 0;
};

class TLinuxDir : public TBaseDir
{
public:
	TLinuxDir(std::string Path);
	bool next(void) override;
	File openFile(std::string s) override;
	String fileName(void) override;
private:
	DIR *_pd;
	dirent *_entry = nullptr;
	std::string _Path;
};

class Dir : public TBaseDir
{
public:
	Dir();
	Dir(const Dir &);
	Dir(std::shared_ptr<TBaseDir> d);
	bool next(void) override;
	File openFile(std::string s) override;
	std::shared_ptr<TBaseDir> GetBaseDir(void) const;
	String fileName(void);
private:
	std::shared_ptr<TBaseDir> _pBaseDir;
};

namespace fs
{
class FS
{
public:
	virtual File open(String s, const char *) = 0;
	bool exists(const char* s);
	//bool exists(std::string &s);
	virtual bool exists(std::__cxx11::basic_string<char> s) = 0;
};

}

typedef fs::FS FS;

class TSPIFFS : public fs::FS
{
public:
	bool begin(void);
	File open(String s, const char *) override;
	bool exists(std::__cxx11::basic_string<char> s) override;
	void remove(int Key);
	void remove(String Key);
	void format(void);
	Dir openDir(std::string d);
	bool rename(std::string From, std::string To);
private:
	std::string GetFullPath(std::string FileName);
	const std::string BASE_DIR = "../data";
};



extern TSPIFFS SPIFFS;

uint32_t spi_flash_get_id(void);
int system_get_flash_size_map(void);

#endif /* SRC_HAL_LINUX_FS_H_ */

/*
 * FS.cpp
 *
 *  Created on: 4 Dec 2025
 *      Author: snh
 */

#include "FS.h"
#include <stdio.h>
#include <unistd.h>
#include "submodules/RFDProxy/RFDLib/File/File.hpp"

TSPIFFS SPIFFS;

String BaseFile::readStringUntil(char x)
{
	int a;
	String Result;

	while ((a = read()) != EOF)
	{
		if (a == x)
		{
			return Result;
		}
		else
		{
			Result += a;
		}
	}

	return Result;
}

/**
 * Read until end of file, return as a string.
 */
String BaseFile::readString(void)
{
	int a;
	String Result;

	while ((a = read()) != EOF)
	{
		Result += (char)a;
	}

	return Result;
}


void BaseFile::print(String s)
{
	write((const uint8_t *)s.c_str(), s.size());
}

void BaseFile::println(String l)
{
	l += "\n";
	print(l);
}

/**
 * Open a linux file
 *
 * @param Name - The name in the ESP's file system.
 * @param Path - The full linux path.
 * @param Mode - The mode
 */
TLinuxFile::TLinuxFile(std::string Name, std::string Path, std::string Mode)
	: _Name(Name)
{
	//printf("Opening %s(%s) with mode %s\n", Name.c_str(), Path.c_str(), Mode.c_str());
	_fp = fopen(Path.c_str(), Mode.c_str());
}

TLinuxFile::operator bool() const
{
	return _fp != nullptr;
}

void TLinuxFile::setTimeout(int t)
{

}

void TLinuxFile::close(void)
{
	//printf("Closing %s\n", _Name.c_str());
	if (_fp != nullptr)
	{
		fclose(_fp);
		_fp = nullptr;
	}
}

bool TLinuxFile::available(void)
{
	return _fp != nullptr && position() < size();
}

/**
 * @return the size of this file, in bytes.
 */
int TLinuxFile::size(void)
{
	if (_fp == nullptr)
	{
		return 0;
	}
	else
	{
		//printf("Position now %d\n", position());
		int p = ftell(_fp);
		fseek(_fp, 0, SEEK_END);
		int Result = ftell(_fp);
		fseek(_fp, p, SEEK_SET);
		//printf("Size is %d, position now %d\n", Result, position());
		return Result;
	}
}

/**
 * Seek to the given position in the file.
 */
void TLinuxFile::seek(int Position)
{
	if (_fp != nullptr)
	{
		fseek(_fp, Position, SEEK_SET);
	}
}

/**
 * @return the position within the file.
 */
int TLinuxFile::position(void)
{
	if (_fp == nullptr)
	{
		return 0;
	}
	else
	{
		return ftell(_fp);
	}
}

/**
 * Read a byte from the file.
 *
 * @return the byte value, or EOF.
 */
int TLinuxFile::read(void)
{
	if (_fp == nullptr)
	{
		return EOF;
	}
	else
	{
		return fgetc(_fp);
	}
}

void TLinuxFile::write(const uint8_t *Buffer, int Length)
{
	if (_fp != nullptr)
	{
		fwrite(Buffer, Length, 1, _fp);
	}
}

String TLinuxFile::name(void)
{
	return _Name;
}


File::File()
{
}

File::File(const File &f)
{
	_pBaseFile = f.GetBaseFile();
}

File::File(std::shared_ptr<BaseFile> f)
	: _pBaseFile(f)
{

}

File::operator bool() const
{
	return _pBaseFile != nullptr && *_pBaseFile;
}

void File::setTimeout(int t)
{
	if (_pBaseFile != nullptr)
	{
		_pBaseFile->setTimeout(t);
	}
}

void File::close(void)
{
	if (_pBaseFile != nullptr)
	{
		_pBaseFile->close();
	}
}

bool File::available(void)
{
	if (_pBaseFile == nullptr)
	{
		return 0;
	}
	else
	{
		return _pBaseFile->available();
	}
}

int File::size(void)
{
	if (_pBaseFile == nullptr)
	{
		return 0;
	}
	else
	{
		return _pBaseFile->size();
	}
}

void File::seek(int n)
{
	if (_pBaseFile != nullptr)
	{
		_pBaseFile->seek(n);
	}
}

int File::position(void)
{
	if (_pBaseFile == nullptr)
	{
		return 0;
	}
	else
	{
		return _pBaseFile->position();
	}
}

int File::read(void)
{
	if (_pBaseFile == nullptr)
	{
		return EOF;
	}
	else
	{
		return _pBaseFile->read();
	}
}

void File::write(const uint8_t *Buffer, int Length)
{
	if (_pBaseFile != nullptr)
	{
		_pBaseFile->write(Buffer, Length);
	}
}

String File::name(void)
{
	if (_pBaseFile == nullptr)
	{
		return String("");
	}
	else
	{
		return _pBaseFile->name();
	}
}

std::shared_ptr<BaseFile> File::GetBaseFile(void) const
{
	return _pBaseFile;
}

TLinuxDir::TLinuxDir(std::string Path)
	: _Path(Path)
{
	_pd = opendir(Path.c_str());
}

/**
 * Move to the next file.
 *
 * @return true if a next file, false if end of dir.
 */
bool TLinuxDir::next(void)
{
	if (_pd == nullptr)
	{
		return false;
	}
	else
	{
		_entry = readdir(_pd);
		return _entry != nullptr;
	}
}

/**
 * Open the current file with the given mode.
 *
 * @param Mode
 * @return the opened filed.
 */
File TLinuxDir::openFile(std::string Mode)
{
	if (_entry == nullptr)
	{
		return File();
	}
	else
	{
		std::string Name(_entry->d_name);
		std::shared_ptr<BaseFile> LF(new TLinuxFile(Name, _Path + "/" + Name, Mode));
		return File(LF);
	}
}

/**
 * @return the name of the current file.
 */
String TLinuxDir::fileName(void)
{
	if (_entry == nullptr)
	{
		return "";
	}
	else
	{
		return String(_entry->d_name);
	}
}

Dir::Dir()
{
}

Dir::Dir(const Dir &d)
{
	_pBaseDir = d.GetBaseDir();
}

Dir::Dir(std::shared_ptr<TBaseDir> d)
	: _pBaseDir(d)
{
}

bool Dir::next(void)
{
	if (_pBaseDir != nullptr)
	{
		return _pBaseDir->next();
	}
	else
	{
		return false;
	}
}

/**
 * Open the current file with the given mode.
 *
 * @param Mode
 * @return the opened filed.
 */
File Dir::openFile(std::string Mode)
{
	if (_pBaseDir != nullptr)
	{
		return _pBaseDir->openFile(Mode);
	}
	else
	{
		return File();
	}
}

std::shared_ptr<TBaseDir> Dir::GetBaseDir(void) const
{
	return _pBaseDir;
}

/**
 * @return the name of the current file.
 */
String Dir::fileName(void)
{
	if (_pBaseDir != nullptr)
	{
		return _pBaseDir->fileName();
	}
	else
	{
		return "";
	}
}

namespace fs
{

bool FS::exists(const char* s)
{
	std::__cxx11::basic_string<char> s2(s);
	return exists(s2);
}

}

bool TSPIFFS::begin(void)
{
	return true;
}

/**
 * Open a file with the given name
 *
 * @param s - File name
 * @param Mode
 * @return the opened file.
 */
File TSPIFFS::open(String s, const char *Mode)
{
	std::shared_ptr<BaseFile> lf(new TLinuxFile(s, GetFullPath(s), std::string(Mode)));

	return File(lf);
}

/**
 * @return whether a file of the given name exists.
 */
bool TSPIFFS::exists(std::__cxx11::basic_string<char> s)
{
	//printf("TSPIFFS::exists %s\n", s.c_str());

	s = GetFullPath(s);
	//printf("\tFull path:  %s\n", s.c_str());

	bool Result = access(s.c_str(), F_OK) == 0;

	//printf("\t%sFound\n", Result ? "" : "Not ");

	return Result;
}

void TSPIFFS::remove(int Key)
{

}

void TSPIFFS::remove(String Key)
{
	//printf("Deleting %s\n", Key.c_str());

	::remove(GetFullPath(Key).c_str());
}

void TSPIFFS::format(void)
{
	RFDLib::File::DeleteDirContents(BASE_DIR);
}

Dir TSPIFFS::openDir(std::string d)
{
	std::shared_ptr<TBaseDir> bd(new TLinuxDir(GetFullPath(d)));
	return Dir(bd);
}

bool TSPIFFS::rename(std::string From, std::string To)
{
	return ::rename(GetFullPath(From).c_str(), GetFullPath(To).c_str()) == 0;
}

/**
 * Get the full path of the file with the given name.
 */
std::string TSPIFFS::GetFullPath(std::string FileName)
{
	if (FileName.size() != 0 && FileName[0] == '/')
	{
		FileName = FileName.substr(1, FileName.size() - 1);
	}

	return BASE_DIR + "/" + FileName;
}

uint32_t spi_flash_get_id(void)
{
	//Only used to display info to the user.
	return 0;
}

int system_get_flash_size_map(void)
{
	//Only used to display info to the user.
	return 0;
}




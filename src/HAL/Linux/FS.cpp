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

String BaseFile::readString(void)
{
	return readStringUntil('\n');
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

TLinuxFile::TLinuxFile(std::string Name, std::string Path)
	: _Name(Name)
{
	_fp = fopen(Path.c_str(), "rw");
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
	if (_fp != nullptr)
	{
		fclose(_fp);
		_fp = nullptr;
	}
}

bool TLinuxFile::available(void)
{
	return _fp != nullptr;
}

int TLinuxFile::size(void)
{
	int p = ftell(_fp);
	fseek(_fp, 0, SEEK_END);
	int Result = ftell(_fp);
	fseek(_fp, p, SEEK_SET);
	return Result;
}

void TLinuxFile::seek(int Position)
{
	fseek(_fp, Position, SEEK_END);
}

int TLinuxFile::position(void)
{
	return ftell(_fp);
}

int TLinuxFile::read(void)
{
	return fgetc(_fp);
}

void TLinuxFile::write(const uint8_t *Buffer, int Length)
{
	fwrite(Buffer, Length, 1, _fp);
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
		return 0;
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

bool TLinuxDir::next(void)
{
	if (_pd == nullptr)
	{
		return false;
	}
	else
	{
		struct dirent *entry = readdir(_pd);
		return entry != nullptr;
	}
}

File TLinuxDir::openFile(std::string s)
{
	std::shared_ptr<BaseFile> LF(new TLinuxFile(s, _Path + "/" + s));
	return File(LF);
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

File Dir::openFile(std::string s)
{
	if (_pBaseDir != nullptr)
	{
		return _pBaseDir->openFile(s);
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

File TSPIFFS::open(String s, const char *)
{
	std::shared_ptr<BaseFile> lf(new TLinuxFile(s, GetFullPath(s)));

	return File(lf);
}

bool TSPIFFS::exists(std::__cxx11::basic_string<char> s)
{
	return access(s.c_str(), F_OK) == 0;
}

void TSPIFFS::remove(int Key)
{

}

void TSPIFFS::remove(String Key)
{
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
	return ::rename(From.c_str(), To.c_str()) == 0;
}

std::string TSPIFFS::GetFullPath(std::string FileName)
{
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





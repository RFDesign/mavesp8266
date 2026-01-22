/*
 * String.cpp
 *
 *  Created on: 20 Nov 2025
 *      Author: snh
 */

#include <cctype>    // std::tolower
#include <algorithm> // std::equal
#include "Arduino.h"


String::String()
{

}

String::String(const char *x)
{
	this->assign(x);
}

String::String(std::string x)
{
	this->assign(x);
}

String::String(size_t n)
{
	this->reserve(n);
}

String String::substring(int start, int size) const
{
	return String(this->substr(start, size));
}

String String::substring(int n) const
{
	return String(this->substr(n, this->size() - n));
}

int String::toInt(void) const
{
	return atoi(c_str());
}

char String::charAt(int Offset) const
{
	return c_str()[Offset];
}

int String::indexOf(const std::string &x) const
{
	return find(x);
}

int String::indexOf(const char x, int n) const
{
	return find(x, n);
}

int String::indexOf(const char x) const
{
	return find(x);
}

void String::trim(void)
{
	int FirstNonSpace = -1;
	int LastNonSpace = 0;

	for (int n = 0; n < size(); n++)
	{
		switch (c_str()[n])
		{
			case ' ':
			case '\t':
				break;
			default:
				if (FirstNonSpace < 0)
				{
					FirstNonSpace = n;
				}
				LastNonSpace = n;
				break;
		}
	}

	if (FirstNonSpace < 0)
	{
		*this = "";
	}
	else
	{
		*this = this->substr(FirstNonSpace, LastNonSpace - FirstNonSpace);
	}
}

bool String::startsWith(std::string s)
{
	return size() >= s.size() && compare(0, s.size(), s) == 0;
}

bool String::endsWith(std::string s) const
{
	return size() >= s.size() && compare(size() - s.size(), s.size(), s) == 0;
}

int String::length(void) const
{
	return size();
}

bool String::equals(const String &x) const
{
	return *this == x;
}

bool String::equalsConstantTime(char *x)
{
	return equals(String(x));
}

bool ichar_equals(char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) ==
           std::tolower(static_cast<unsigned char>(b));
}

bool iequals(const std::string& a, const std::string& b)
{
    return a.size() == b.size() &&
           std::equal(a.begin(), a.end(), b.begin(), ichar_equals);
}

bool String::equalsIgnoreCase(const char *x)
{
	std::string b(x);
	return iequals(*this, b);
}

bool String::equalsIgnoreCase(const String x)
{
	return iequals(*this, x);
}

bool String::reserve(size_t n)
{
	this->resize(n);
	return true;
}

void String::replace(const char *x, const char *y)
{
	String a(x);
	String b(y);
	replace(a,b);
}

void String::replace(String x, String y)
{
	size_t n = find(x);
	if (n != std::string::npos)
	{
		std::string x = *this;

		x.replace((int)n, (int)y.size(), y);
		*this = x;
	}
}

String emptyString;

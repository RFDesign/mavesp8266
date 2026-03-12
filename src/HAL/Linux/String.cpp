/*
 * String.cpp
 *
 *  Created on: 20 Nov 2025
 *      Author: snh
 */

#include <cctype>    // std::tolower
#include <algorithm> // std::equal
#include <string>
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

/**
 * Create a new string which is a text representation of the given number.  Base 10.
 */
String::String(int NumberToConvertToString)
{
	this->assign(std::to_string(NumberToConvertToString));
}

/**
 * Create a new string which is a text representation of the given number.  Base 16.
 */
String::String(int NumberToConvertToString, int Base)
{
	if (Base == DEC)
	{
		this->assign(std::to_string(NumberToConvertToString));
	}
	else
	{
		char Temp[100];
		sprintf(Temp, "%X", NumberToConvertToString);
		this->assign(Temp);
	}
}

/**
 * Get a substring
 *
 * @param start - start index
 * @param end - end index
 * @return sub string
 */
String String::substring(int start, int end) const
{
	return String(this->substr(start, end - start));
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

/**
 * Check whether this string ends with the given string.
 *
 * @param s - The given string to compare the end to.
 * @return true if this string ends in the given s, otherwise false.
 *
 */
bool String::endsWith(std::string s) const
{
	const char *This = c_str();
	const char *Other = s.c_str();

	int ThisLength = strlen(This);
	int OtherLength = strlen(Other);

	if (ThisLength >= OtherLength)
	{
		for (int n = 0; n < OtherLength; n++)
		{
			if (Other[n] != This[n + ThisLength - OtherLength])
			{
				return false;
			}
		}

		return true;
	}
	else
	{
		return false;
	}

	//return (size() >= s.size()) && (compare(size() - s.size(), s.size(), s) == 0);
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

/**
 * Replace all instances of x with y
 */
void String::replace(String x, String y)
{
	if (x.size() == 0)
	{
		return;
	}

	size_t n = 0;
	while ((n = this->find(x, n)) != std::string::npos)
	{
		std::string tmp = *this;
		tmp.replace(n, x.size(), y);
		*this = tmp;
		n += y.size();
	}
}

String emptyString;

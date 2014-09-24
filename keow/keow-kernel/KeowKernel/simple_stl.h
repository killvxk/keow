/*
 * Copyright 2005 Paul Walker
 *
 * GNU General Public License
 * 
 * This file is part of: Kernel Emulation on Windows (keow)
 *
 * Keow is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Keow is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Keow; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

//Simple STL library (see comment in includes.h)
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SIMPLE_STL_H__4305518F_D005_41B7_8993_8F7E70C9A0AC__INCLUDED_)
#define AFX_SIMPLE_STL_H__4305518F_D005_41B7_8993_8F7E70C9A0AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "string.h"

namespace std {

/////////////////////////////////////////////////////////////////

class string
{
public:
	string();
	string(const string& str);
	string(const char *str);
	~string();

	int length() const;
	bool empty() const;
	void reserve(int size);

	string& operator += (char c);
	string& operator += (const char * str);
	string& operator += (const string& str);
	string& operator = (const char * str);
	string& operator = (const string& str);

	char operator [] (int pos);

	bool operator == (const char * str) const;
	bool operator == (const string& str) const;
	bool operator != (const char * str) const;
	bool operator != (const string& str) const;
	int compare(const char * str) const;
	int compare(const string& str) const;

	const char * c_str() const;
	int find(char c, int pos=0) const;
	string substr(int pos, int len) const;
	string substr(int pos) const;

protected:
	char * m_pChars;
	int m_nLen, m_nCapacity;

	void null_terminate();
};

inline string::string()
{
	m_pChars=0;
	m_nLen=m_nCapacity=0;
}
inline string::string(const string& str)
{
	*this = str;
}
inline string::string(const char *str)
{
	*this = str;
}

inline string::~string()
{
	if(m_pChars)
		delete m_pChars;
}

inline void string::null_terminate()
{
	reserve(m_nLen+1);
	m_pChars[m_nLen] = 0;
}

inline bool string::empty() const
{
	return m_pChars==0 || m_nLen==0;
}
inline int string::length() const
{
	return m_nLen;
}
inline void string::reserve(int size)
{
	if(m_nCapacity >= size)
		return;

	while(m_nCapacity < size)
		m_nCapacity += 128; //what to do here? try something!

	char * p2 = new char[m_nCapacity];
	if(m_pChars)
	{
		memcpy(p2,m_pChars, m_nLen);
		delete m_pChars;
	}
	m_pChars = p2;
}

inline string& string::operator+=(char c)
{
	reserve(m_nLen+2);
	m_pChars[m_nLen] = c;
	m_nLen++;
	null_terminate();
	return *this;
}
inline string& string::operator += (const char * str)
{
	int len = strlen(str);
	reserve(m_nLen+len);
	memcpy(&m_pChars[m_nLen], str, len);
	m_nLen+=len;
	null_terminate();
	return *this;
}
inline string& string::operator += (const string& str)
{
	if(str.empty())
		return *this;
	reserve(m_nLen+str.m_nLen);
	memcpy(&m_pChars[m_nLen], str.m_pChars, str.m_nLen);
	m_nLen+=str.m_nLen;
	null_terminate();
	return *this;
}

inline const char * string::c_str() const
{
	return m_pChars;
}

inline int string::find(char c, int pos) const
{
	if(empty())
		return -1;

	for(int n=pos; n<m_nLen; ++n)
	{
		if(m_pChars[n]==c)
			return n;
	}
	return -1; //not found
}

inline string& string::operator = (const char * str)
{
	int len = strlen(str);
	reserve(len);
	memcpy(m_pChars, str, len);
	m_nLen = len;
	null_terminate();
	return *this;
}
inline string& string::operator = (const string& str)
{
	reserve(str.m_nLen);
	memcpy(m_pChars, str.m_pChars, str.m_nLen);
	m_nLen = str.m_nLen;
	null_terminate();
	return *this;
}

inline string string::substr(int pos, int len) const
{
	string s;

	if(pos<m_nLen)
	{
		if(pos+len > m_nLen)
			len = m_nLen - pos;
		s.reserve(len);
		memcpy(s.m_pChars, &m_pChars[pos], len);
		s.m_nLen = len;
		s.null_terminate();
	}

	return s;
}
inline string string::substr(int pos) const
{
	return substr(pos,m_nLen);
}

inline bool string::operator == (const char * str) const
{
	return compare(str)==0;
}
inline bool string::operator == (const string& str) const
{
	return compare(str)==0;
}
inline bool string::operator != (const char * str) const
{
	return compare(str)!=0;
}
inline bool string::operator != (const string& str) const
{
	return compare(str)!=0;
}

inline int string::compare(const char * str) const
{
	return strcmp(c_str(), str);
}
inline int string::compare(const string& str) const
{
	return strcmp(c_str(), str.c_str());
}

inline char string::operator [] (int pos)
{
	if(pos>=m_nLen)
		return 0;
	return m_pChars[pos];
}

/////////////////////////////////////////////////////////////////

template<class T>
class list
{
protected:
	class node
	{
		friend list;

		node *prev, *next;
		T value;
	};
	node *head, *tail;

	static node *list_eof;

public:
	class iterator
	{
	private:
		friend list;
		iterator();
		node *m_pNode;

	public:
		T& operator*();
		iterator& operator=(iterator& other);
		bool operator==(iterator& other);
		bool operator!=(iterator& other);
		iterator& operator++();
	};

public:
	list& push_back(T element);
	T pop_back();
	void erase(iterator it);
	void erase(T value);
	void clear();
	bool empty() const;
	int size() const;

	iterator begin() const;
	iterator end() const;
};

/////////////////////////////////////////////////////////////////

} //namespace

#endif // !defined(AFX_SIMPLE_STL_H__4305518F_D005_41B7_8993_8F7E70C9A0AC__INCLUDED_)

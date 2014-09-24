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

//Simple String implementation (like STL) (see comment in includes.h)
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_STRING_H__4305518F_D005_41B7_8993_8F7E70C9A0AC__INCLUDED_)
#define AFX_STRING_H__4305518F_D005_41B7_8993_8F7E70C9A0AC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "String.h"


//TODO: make string copy-on-write

class string
{
public:
	string();
	string(const string& str);
	string(const char *str);
	~string();

	DWORD hash() const;
	int length() const;
	int capacity() const;
	bool empty() const;
	void reserve(int size);

	string operator + (const string& str) const;
	string& operator += (char c);
	string& operator += (const char * str);
	string& operator += (const string& str);
	string& operator = (const char * str);
	string& operator = (const string& str);

	char& operator [] (int pos);

	operator const char *() const; 

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

	char * GetBuffer(int size);
	void ReleaseBuffer();

	static string format(const char * fmt, ...);

protected:
	char * m_pChars;
	int m_nLen, m_nCapacity;

	void null_terminate();
};

/////////////////////////////////////////////////////////////////

#endif // !defined(AFX_STRING_H__4305518F_D005_41B7_8993_8F7E70C9A0AC__INCLUDED_)

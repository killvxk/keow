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

// Utils.h: interface for the Utils class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UTILS_H__23B86EDD_923F_42A7_94E3_68A42DB40A3D__INCLUDED_)
#define AFX_UTILS_H__23B86EDD_923F_42A7_94E3_68A42DB40A3D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


void ktrace(const char *format, ...);
void halt();
void ParseCommandLine(LPSTR lpCmdLine, list<string> &lst);


const int SIZE4k = 4*1024;
const int SIZE64k = 64*1024;

//unix time base
extern ULARGE_INTEGER Time1Jan1970;
//conversion (1sec <--> 100ns)
#define SECOND_TO_100NS(sec)  (((unsigned __int64)sec)*10000000L)
#define NS100_TO_SECOND(ns)  (((unsigned __int64)ns)/10000000L)
#define FILETIME_TO_TIME_T(t) (unsigned long)(NS100_TO_SECOND( ((ULARGE_INTEGER*)&t)->QuadPart - Time1Jan1970.QuadPart ))
#define TIME_T_TO_FILETIME(tt,ft) ((ULARGE_INTEGER*)&ft)->QuadPart = (Time1Jan1970.QuadPart + SECOND_TO_100NS(tt))

//c++ helper
#define instanceof(var,type) (dynamic_cast<type*>(var) != 0)

//calculate byte offsets
#define offset_of(base,member) ( ((DWORD)&member) - ((DWORD)&base) )


//information about threads
struct ThreadInfo
{
	DWORD dwThreadId;
	HANDLE hThread;

	DWORD SegGs;

	ThreadInfo(DWORD id, HANDLE h)
		: dwThreadId(id), hThread(h), SegGs(0)
	{
	}
};

#endif // !defined(AFX_UTILS_H__23B86EDD_923F_42A7_94E3_68A42DB40A3D__INCLUDED_)

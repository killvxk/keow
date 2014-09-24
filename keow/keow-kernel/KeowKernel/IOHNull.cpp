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

// IOHNull.cpp: implementation of the 
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "IOHNull.h"

//////////////////////////////////////////////////////////////////////

IOHNull::IOHNull()
{
	m_hRemoteHandle = INVALID_HANDLE_VALUE;
	Open(GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING, 0);
}

IOHNull::~IOHNull()
{
	Close();
}


bool IOHNull::Open(DWORD win32access, DWORD win32share, DWORD disposition, DWORD flags)
{
	Close();

	HANDLE h = CreateFile("NUL:", GENERIC_READ|GENERIC_WRITE, 0, 0, OPEN_EXISTING, 0, 0);

	DuplicateHandle(GetCurrentProcess(), h,
		P->m_hProcess, &m_hRemoteHandle,
		0, FALSE, DUPLICATE_SAME_ACCESS|DUPLICATE_CLOSE_SOURCE);
	return true;
}

bool IOHNull::Close()
{
	if(m_hRemoteHandle != INVALID_HANDLE_VALUE)
		SysCallDll::CloseHandle(m_hRemoteHandle);
	m_hRemoteHandle = INVALID_HANDLE_VALUE;
	return true;
}


IOHandler* IOHNull::Duplicate()
{
	IOHNull * pC = new IOHNull();
	return pC;
}

bool IOHNull::Stat64(linux::stat64 * s)
{
	if(!s)
		return false;

	IOHandler::BasicStat64(s, linux::S_IFCHR);

	return true;
}


DWORD IOHNull::ioctl(DWORD request, DWORD data)
{
	return 0;
}



bool IOHNull::Read(void* address, DWORD size, DWORD *pRead)
{
	SysCallDll::ZeroMem(address, size);
	*pRead = size;
	return true;
}


bool IOHNull::Write(void* address, DWORD size, DWORD *pWritten)
{
	//swallow it
	*pWritten = size;
	return true;
}


bool IOHNull::CanRead()
{
	//never - can't get data from /dev/null
	return false;
}

bool IOHNull::CanWrite()
{
	//always accept data
	return true;
}

bool IOHNull::HasException()
{
	//always ok
	return false;
}


__int64 IOHNull::Length()
{
	return 0;
}

__int64 IOHNull::Seek(__int64 offset, DWORD method)
{
	return -1;
}

void IOHNull::Truncate()
{
	//do nothing
}

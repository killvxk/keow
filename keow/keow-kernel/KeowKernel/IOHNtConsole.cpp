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

// IOHNtConsole.cpp: implementation of the IOHNtConsole class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "IOHNtConsole.h"


//////////////////////////////////////////////////////////////////////

IOHNtConsole::IOHNtConsole(DevConsole* pConsole)
{
	m_pConsole = pConsole;

	m_hRemoteConsoleRead = m_hRemoteConsoleWrite = INVALID_HANDLE_VALUE;

	Open(GENERIC_READ|GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE, OPEN_EXISTING, 0);
}

IOHNtConsole::~IOHNtConsole()
{
	Close();
}


IOHandler * IOHNtConsole::Duplicate()
{
	IOHNtConsole * pC = new IOHNtConsole(m_pConsole);
	return pC;
}

bool IOHNtConsole::Open(DWORD win32access, DWORD win32share, DWORD disposition, DWORD flags)
{
	Close();

	DuplicateHandle(GetCurrentProcess(), m_pConsole->m_hConsoleRead,
		P->m_hProcess, &m_hRemoteConsoleRead,
		0, FALSE, DUPLICATE_SAME_ACCESS);

	DuplicateHandle(GetCurrentProcess(), m_pConsole->m_hConsoleWrite,
		P->m_hProcess, &m_hRemoteConsoleWrite,
		0, FALSE, DUPLICATE_SAME_ACCESS);

	return true;
}

bool IOHNtConsole::Close()
{
	if(m_hRemoteConsoleRead != INVALID_HANDLE_VALUE)
		SysCallDll::CloseHandle(m_hRemoteConsoleRead);
	if(m_hRemoteConsoleWrite != INVALID_HANDLE_VALUE)
		SysCallDll::CloseHandle(m_hRemoteConsoleWrite);

	m_hRemoteConsoleRead = m_hRemoteConsoleWrite = INVALID_HANDLE_VALUE;

	return true;
}


bool IOHNtConsole::Stat64(linux::stat64 * s)
{
	if(!s)
		return false;

	IOHandler::BasicStat64(s, linux::S_IFCHR); //console is a character device

	return true;
}

DWORD IOHNtConsole::ioctl(DWORD request, DWORD data)
{
	return m_pConsole->ioctl(request, data);
}


bool IOHNtConsole::Read(void* address, DWORD size, DWORD *pRead)
{
	if(m_Flags&linux::O_NONBLOCK)
	{
		//non-blocking - ensure we can do the read before doing it
		DWORD dwBytes;
		dwBytes = SysCallDll::PeekAvailablePipe(m_hRemoteConsoleRead);
		if(dwBytes < size)
		{
			if(dwBytes==0)
			{
				if(SysCallDll::GetLastError()==ERROR_BROKEN_PIPE)
				{
					//EOF
					*pRead = 0;
					return true;
				}
			}
			//can't read without blocking
			return false;
		}
	}

	//read
	*pRead = SysCallDll::ReadFile(m_hRemoteConsoleRead, address, size);
	if(*pRead==0)
	{
		if(SysCallDll::GetLastError()==ERROR_BROKEN_PIPE)
		{
			//EOF
			return true;
		}

		//failed read
		return false;
	}
	return true;
}


bool IOHNtConsole::Write(void* address, DWORD size, DWORD *pWritten)
{
	*pWritten = SysCallDll::WriteFile(m_hRemoteConsoleWrite, address, size);
	return *pWritten!=0;
}


bool IOHNtConsole::CanRead()
{
	//ok if we are not at eof
	DWORD dwAvail = SysCallDll::PeekAvailablePipe(m_hRemoteConsoleRead);
	return dwAvail!=0;
}

bool IOHNtConsole::CanWrite()
{
	//TODO: how will we know?
	return true;
}

bool IOHNtConsole::HasException()
{
	//TODO: what could this be?
	return false;
}

__int64 IOHNtConsole::Length()
{
	return 0;
}

__int64 IOHNtConsole::Seek(__int64 offset, DWORD method)
{
	return -1;
}

void IOHNtConsole::Truncate()
{
	//do nothing
}

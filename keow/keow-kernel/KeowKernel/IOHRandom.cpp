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

// IOHRandom.cpp: implementation of the 
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "IOHRandom.h"

//////////////////////////////////////////////////////////////////////

IOHRandom::IOHRandom(bool urandom)
{
	m_bIsURandom = urandom;
	m_LastGenerated = GetTickCount() ^ (DWORD)this; //not very random!
}

IOHRandom::~IOHRandom()
{
}


bool IOHRandom::Open(DWORD win32access, DWORD win32share, DWORD disposition, DWORD flags)
{
	return true;
}

bool IOHRandom::Close()
{
	return true;
}


IOHandler* IOHRandom::Duplicate()
{
	IOHRandom * pC = new IOHRandom(m_bIsURandom);
	return pC;
}

bool IOHRandom::Stat64(linux::stat64 * s)
{
	if(!s)
		return false;

	IOHandler::BasicStat64(s, linux::S_IFCHR);

	return true;
}


DWORD IOHRandom::ioctl(DWORD request, DWORD data)
{
	return 0;
}



bool IOHRandom::Read(void* address, DWORD size, DWORD *pRead)
{
	ADDR addr = (ADDR)address;
	*pRead = size;
	while(size != 0)
	{
		int block = size>SIZE4k?SIZE4k:size;
		GenerateNumbers(block);
		P->WriteMemory(addr, block, m_RandomBytes);
		addr += block;
		size -= block;
	}
	return true;
}


bool IOHRandom::Write(void* address, DWORD size, DWORD *pWritten)
{
	//swallow it
	*pWritten = size;
	return true;
}


bool IOHRandom::CanRead()
{
	//always
	return true;
}

bool IOHRandom::CanWrite()
{
	//cannot write to random number generator
	return false;
}

bool IOHRandom::HasException()
{
	//always ok
	return false;
}


__int64 IOHRandom::Length()
{
	return 0;
}

__int64 IOHRandom::Seek(__int64 offset, DWORD method)
{
	return -1;
}

void IOHRandom::Truncate()
{
	//do nothing
}


// ensure at least this many numbers in the buffer
void IOHRandom::GenerateNumbers(int needed)
{
#ifdef _DEBUG
	if(needed > sizeof(m_RandomBytes))
		DebugBreak();
#endif

	WORD * pRnd = (WORD*)m_RandomBytes;
	for(int i=0; i<needed; ++pRnd,i+=sizeof(*pRnd))
	{
		// if(m_bIsURandom)  should be better random - how?
		unsigned int seed = m_LastGenerated * GetTickCount() * g_pKernelTable->m_ForksSinceBoot * g_pKernelTable->m_Processes.size();
		srand(seed);
		m_LastGenerated = rand();

		*pRnd = m_LastGenerated;
	}
}

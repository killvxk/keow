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

// IOHRandom.h: interface for the IOHRandom class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IOHRandom_H__A82590CD_8CD7_4759_B40F_0563D597E0C6__INCLUDED_)
#define AFX_IOHRandom_H__A82590CD_8CD7_4759_B40F_0563D597E0C6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IOHandler.h"

class IOHRandom : public IOHandler  
{
public:
	IOHRandom(bool urandom);
	virtual ~IOHRandom();

	virtual bool Open(DWORD win32access, DWORD win32share, DWORD disposition, DWORD flags);
	virtual bool Close();
	virtual DWORD ioctl(DWORD request, DWORD data);

	virtual bool Read(void* address, DWORD size, DWORD *pRead);
	virtual bool Write(void* address, DWORD size, DWORD *pWritten);

	virtual __int64 Length();
	virtual __int64 Seek(__int64 offset, DWORD method);
	virtual void Truncate();

	virtual IOHandler* Duplicate();

	virtual bool Stat64(linux::stat64 * s);

	virtual bool CanRead();
	virtual bool CanWrite();
	virtual bool HasException();

private:
	BYTE m_RandomBytes[SIZE4k];
	int m_LastGenerated;
	bool m_bIsURandom;

	void GenerateNumbers(int needed);
};

#endif // !defined(AFX_IOHRandom_H__A82590CD_8CD7_4759_B40F_0563D597E0C6__INCLUDED_)

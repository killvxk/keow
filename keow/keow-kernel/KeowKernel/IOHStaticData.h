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

// IOHStaticData.h: interface for the IOHStaticData class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_IOHSTATICDATA_H__F88B0536_91B6_452C_AD1B_075A41EFCECD__INCLUDED_)
#define AFX_IOHSTATICDATA_H__F88B0536_91B6_452C_AD1B_075A41EFCECD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IOHandler.h"

class IOHStaticData : public IOHandler  
{
public:
	enum DataType {
		File,
		Directory
	};

	IOHStaticData(Path& path, DataType type, bool Refreshable);
	virtual ~IOHStaticData();

	void AddData(const char * text);
	void AddData(void * pData, int length);
	void AddData(linux::dirent64 de);

public:
	virtual bool Open(DWORD win32access, DWORD win32share, DWORD disposition, DWORD flags);
	virtual bool Close();
	virtual DWORD ioctl(DWORD request, DWORD data);
	virtual int GetDirEnts64(linux::dirent64 *de, int maxbytes);

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

protected:
	void RefreshData();
	bool m_bRefreshable;

	BYTE * m_pData;
	int m_DataLength;
	int m_nFilePointer;

	Path m_Path;
	DataType m_DataType;
};

#endif // !defined(AFX_IOHSTATICDATA_H__F88B0536_91B6_452C_AD1B_075A41EFCECD__INCLUDED_)

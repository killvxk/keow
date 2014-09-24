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

// MountPoint.h: interface for the MountPoint class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_MOUNTPOINT_H__FA34E576_A49E_41D5_9108_47937854CEDD__INCLUDED_)
#define AFX_MOUNTPOINT_H__FA34E576_A49E_41D5_9108_47937854CEDD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Filesystem;

class MountPoint  
{
public:
	bool Unmount();

	static MountPoint* Mount( Path& UnixMountPoint, string sDestination, Filesystem* pFS, DWORD mountflags, BYTE * pData, int nDataLen);

	string GetDestination()
	{
		return m_strDestination;
	}
	Path GetUnixMountPoint()
	{
		return m_UnixMountPoint;
	}

	Filesystem* GetFilesystem()
	{
		return m_pFileSystem;
	}

	string GetOptions()
	{
		return m_strOptions;
	}

protected:
	friend Filesystem;

	MountPoint();
	virtual ~MountPoint();

	Path	m_UnixMountPoint;
	string	m_strDestination;
	string  m_strOptions;
	Filesystem * m_pFileSystem;
	DWORD	m_dwMountFlags;
	void * m_pData;
	int m_nDataLength;
};

#endif // !defined(AFX_MOUNTPOINT_H__FA34E576_A49E_41D5_9108_47937854CEDD__INCLUDED_)

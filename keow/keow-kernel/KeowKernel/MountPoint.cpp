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

// MountPoint.cpp: implementation of the MountPoint class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "MountPoint.h"

//////////////////////////////////////////////////////////////////////

MountPoint::MountPoint()
{

}

MountPoint::~MountPoint()
{
	//no longer need the handler
	delete m_pFileSystem;

	delete m_pData;
}


MountPoint* MountPoint::Mount( Path& UnixMountPoint, string sDestination, Filesystem* pFS, DWORD mountflags, BYTE * pData, int nDataLen)
{
	MountPoint * pMP = new MountPoint();

	pMP->m_UnixMountPoint = UnixMountPoint;
	pMP->m_strDestination = sDestination;

	pMP->m_pFileSystem = pFS;
	pMP->m_pFileSystem->SetAssociatedMount(*pMP);

	pMP->m_dwMountFlags = mountflags;

	pMP->m_pData = new BYTE[nDataLen];
	pMP->m_nDataLength = nDataLen;
	memcpy(pMP->m_pData, pData, nDataLen);

	//add to the kernel
	g_pKernelTable->m_MountPoints.push_back(pMP);

	return pMP;
}

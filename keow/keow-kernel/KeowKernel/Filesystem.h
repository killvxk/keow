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

// Filesystem.h: interface for the Filesystem class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILESYSTEM_H__4BFEC0CD_0A6A_42B5_9BB8_445350427B3E__INCLUDED_)
#define AFX_FILESYSTEM_H__4BFEC0CD_0A6A_42B5_9BB8_445350427B3E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Filesystem  
{
public:
	virtual const char * Name() = 0;

	virtual void SetAssociatedMount(MountPoint& mp);

	virtual IOHandler * CreateIOHandler(Path& path) = 0;
	virtual string GetPathSeperator() = 0;
	virtual bool IsSymbolicLink(string& strPath) = 0;
	virtual string GetLinkDestination(string& strPath) = 0;
	virtual bool IsRelativePath(string& strPath) = 0;

	Filesystem();
	virtual ~Filesystem();

protected:
	MountPoint * m_pAssociatedMountPoint;
};

#endif // !defined(AFX_FILESYSTEM_H__4BFEC0CD_0A6A_42B5_9BB8_445350427B3E__INCLUDED_)

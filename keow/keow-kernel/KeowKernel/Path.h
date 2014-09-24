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

// Path.h: interface for the Path class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_PATH_H__318FAE4B_1C88_435D_A35C_D1849ACD47AA__INCLUDED_)
#define AFX_PATH_H__318FAE4B_1C88_435D_A35C_D1849ACD47AA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


class MountPoint;
class Filesystem;

class Path  
{
public:
	Path(const Path& other);
	Path(bool FollowSymLinks = true);
	Path(string UnixPath, bool FollowSymLinks = true);
	Path(LPCSTR UnixPath, bool FollowSymLinks = true);
	virtual ~Path();

	Path& operator=(const Path& other);

	bool operator==(const Path& other) const;

	void SetUnixPath(string path);
	void AppendUnixPath(string unixp);

	bool EqualsPartialPath(const Path& other, int ElementCount) const;

	void FollowSymLinks(bool follow);

	bool IsSymbolicLink();

	Filesystem * GetFinalFilesystem();

	int GetElementCount();
	string GetUnixPathElement(int count);

	string GetUnixPath();
	string GetWin32Path();
	string GetPathInFilesystem();

protected:
	typedef list<const string> ElementList;

	static void AppendPath(ElementList& list, string path);
	static string JoinList(const Path::ElementList& list, char delimiter);

	void TranverseMountPoints();
	string GetFinalPath();

protected:
	ElementList m_PathStack;
	MountPoint * m_pFinalMountPoint;
	string m_strMountRealPath, m_strPathInMountPoint;
	string m_strActualUnixPath;
	bool m_FollowSymLinks;
	bool m_NeedCalculation;
	string m_strWin32Path;
};

#endif // !defined(AFX_PATH_H__318FAE4B_1C88_435D_A35C_D1849ACD47AA__INCLUDED_)

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

// FilesystemGenericStatic is an abstract class that helps to implement
// a static filesystem such as /proc
// It allows inheriting filesystems to define a virtual directory
// structure, with specific routines designated as handling specific
// "files" within it.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FILESYSTEMGENERICSTATIC_H__F05AD407_0077_4FCF_B679_EE569DFC5F22__INCLUDED_)
#define AFX_FILESYSTEMGENERICSTATIC_H__F05AD407_0077_4FCF_B679_EE569DFC5F22__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


typedef list<linux::dirent64> DirEnt64List;

typedef void (*FilesystemGenericStaticLister0)(DirEnt64List& lst);
typedef void (*FilesystemGenericStaticLister1)(DirEnt64List& lst, const char * param1);

typedef IOHandler* (*FilesystemGenericStaticGetHandler0)(Path& path);
typedef IOHandler* (*FilesystemGenericStaticGetHandler1)(Path& path, const char * param1);
typedef IOHandler* (*FilesystemGenericStaticGetHandler2)(Path& path, const char * param1, const char * param2);


class FilesystemGenericStatic : public Filesystem
{
protected:
	FilesystemGenericStatic(const char * FsName);
	virtual ~FilesystemGenericStatic();

	void AddLister(const char * pattern, FilesystemGenericStaticLister0 lister);
	void AddLister(const char * pattern, FilesystemGenericStaticLister1 lister);

	void AddFile(const char * pattern, FilesystemGenericStaticGetHandler0 handler);
	void AddFile(const char * pattern, FilesystemGenericStaticGetHandler1 handler);
	void AddFile(const char * pattern, FilesystemGenericStaticGetHandler2 handler);

private:
	class PathMappingNode;
	typedef list<PathMappingNode*> PathMappingList;

	class PathMappingNode {
	public:
		PathMappingNode() {
			NumParams = 0;
			lister.Lister0 = NULL;
			handler.GetHandlerProc0 = NULL;
		}

		string PathPattern;
		PathMappingList children;

		int NumParams; //number of * wildcards in path to reach here

		//for when this is a file (leaf on the tree, no children)
		struct {
			union {
				FilesystemGenericStaticGetHandler0 GetHandlerProc0;
				FilesystemGenericStaticGetHandler1 GetHandlerProc1;
				FilesystemGenericStaticGetHandler2 GetHandlerProc2;
			};
		} handler;

		//for when this is a directory (has children)
		union {
			FilesystemGenericStaticLister0 Lister0;
			FilesystemGenericStaticLister1 Lister1;
		} lister;
	};

	PathMappingNode * m_pPathMappingRoot;
	const char * m_pFsName;

private:
	void DestroyNode(PathMappingNode * node);
	void AddFileInternal(const char * pattern, int wildcards, FilesystemGenericStaticGetHandler0 handler);
	void AddListerInternal(const char * pattern, int wildcards, FilesystemGenericStaticLister0 lister);

public:
	virtual const char * Name();

	virtual IOHandler * CreateIOHandler(Path& path);
	virtual string GetPathSeperator();
	virtual bool IsSymbolicLink(string& strPath);
	virtual string GetLinkDestination(string& strPath);
	virtual bool IsRelativePath(string& strPath);
};

#endif // !defined(AFX_FILESYSTEMGENERICSTATIC_H__F05AD407_0077_4FCF_B679_EE569DFC5F22__INCLUDED_)

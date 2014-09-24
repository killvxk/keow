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

// FilesystemGenericStatic.cpp: implementation of the FilesystemGenericStatic class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "FilesystemGenericStatic.h"
#include "IOHStaticData.h"

//////////////////////////////////////////////////////////////////////

FilesystemGenericStatic::FilesystemGenericStatic(const char * FsName)
{
	m_pFsName = FsName;
	m_pPathMappingRoot = new PathMappingNode();
}

FilesystemGenericStatic::~FilesystemGenericStatic()
{
	//destroy the internal tree
	DestroyNode(m_pPathMappingRoot);
}


void FilesystemGenericStatic::AddLister(const char * pattern, FilesystemGenericStaticLister0 lister)
{
	AddListerInternal(pattern, 0, lister);
}
void FilesystemGenericStatic::AddLister(const char * pattern, FilesystemGenericStaticLister1 lister)
{
	AddListerInternal(pattern, 1, (FilesystemGenericStaticLister0)lister);
}

void FilesystemGenericStatic::AddFile(const char * pattern, FilesystemGenericStaticGetHandler0 handler)
{
	AddFileInternal(pattern, 0, handler);
}
void FilesystemGenericStatic::AddFile(const char * pattern, FilesystemGenericStaticGetHandler1 handler)
{
	AddFileInternal(pattern, 1, (FilesystemGenericStaticGetHandler0)handler);
}
void FilesystemGenericStatic::AddFile(const char * pattern, FilesystemGenericStaticGetHandler2 handler)
{
	AddFileInternal(pattern, 2, (FilesystemGenericStaticGetHandler0)handler);
}



void FilesystemGenericStatic::AddFileInternal(const char * pattern, int wildcards, FilesystemGenericStaticGetHandler0 handler)
{
	//add the file/handler as a leaf on the tree

	Path path = pattern;
	PathMappingNode * node = m_pPathMappingRoot;
	int depth=0;

	while(depth < path.GetElementCount()-1)
	{
		string want = path.GetUnixPathElement(depth);

		PathMappingList::iterator it;
		for(it=node->children.begin();
		    it!=node->children.end();
			++it)
		{
			if((*it)->PathPattern == want)
			{
				node = *it;
				++depth;
				break;
			}
		}

		if(it==node->children.end())
		{
			//need to add an intermediate node
			PathMappingNode * p2 = new PathMappingNode();
			p2->PathPattern = want;

			node->children.push_back(p2);
			node = p2;
			++depth;
		}
	}

	//last element - we are adding this
	PathMappingNode * p2 = new PathMappingNode();
	p2->PathPattern = path.GetUnixPathElement(depth);
	p2->NumParams = wildcards;
	p2->handler.GetHandlerProc0 = handler;

	node->children.push_back(p2);
}

void FilesystemGenericStatic::AddListerInternal(const char * pattern, int wildcards, FilesystemGenericStaticLister0 lister)
{
	//find(/add) node and specify it's lister

	Path path = pattern;
	PathMappingNode * node = m_pPathMappingRoot;
	int depth=0;

	while(depth < path.GetElementCount())
	{
		string want = path.GetUnixPathElement(depth);

		PathMappingList::iterator it;
		for(it=node->children.begin();
		    it!=node->children.end();
			++it)
		{
			if((*it)->PathPattern == want)
			{
				node = *it;
				++depth;
				break;
			}
		}
		if(it==node->children.end())
		{
			//need to add an intermediate node
			PathMappingNode * p2 = new PathMappingNode();
			p2->PathPattern = want;

			node->children.push_back(p2);
			node = p2;
			++depth;
		}
	}

	//node is the one that this lister supplies elements to
	node->lister.Lister0 = lister;
	node->NumParams = wildcards;
}


void FilesystemGenericStatic::DestroyNode(PathMappingNode * node)
{
	if(!node)
		return;

	while(!node->children.empty())
	{
		PathMappingNode *pm = node->children.pop_back();
		DestroyNode(pm);
	}
	delete node;
}


IOHandler * FilesystemGenericStatic::CreateIOHandler(Path& path)
{
	//determine the path within this filesystem,
	Path FsPath = path.GetPathInFilesystem();

	//look up a data supplier routine for this path

	PathMappingNode * node = m_pPathMappingRoot;
	int depth=0;
	list<string> wildcards;

	while(depth < FsPath.GetElementCount())
	{
		string want = FsPath.GetUnixPathElement(depth);

		PathMappingNode * found = NULL;
		PathMappingList::iterator it;
		for(it=node->children.begin();
		    it!=node->children.end();
			++it)
		{
			if((*it)->PathPattern == "*")
			{
				found = *it;
				//this is a wildcard match, keep searching as a specific named match has higher priority
			}

			if((*it)->PathPattern == want)
			{
				found = *it;
				//a definite match, stop searching
				break;
			}
		}
		if(found==NULL)
		{
			//no matching element
			ktrace("Implement GenericStatic mapping for: %s: %s\n", m_pFsName, FsPath.GetUnixPath());
			return NULL;
		}


		if(found->PathPattern == "*")
			wildcards.push_back(want);
		node = found;
		++depth;
	}


	//found a node that matches the path
	//is it a file or do we need to list a directories contents?

	if(node->children.empty()
	&& node->lister.Lister0==NULL)
	{
		//a "file"

		switch(node->NumParams)
		{
		case 0:
			return node->handler.GetHandlerProc0(path);
		case 1:
			return node->handler.GetHandlerProc1(path, wildcards[0]);
		case 2:
			return node->handler.GetHandlerProc2(path, wildcards[0], wildcards[1]);

		default:
			ktrace("Implement GenericStatic handler for params: %d\n", node->NumParams);
			return NULL;
		}
	}
	else
	{
		//a "directory"

		IOHStaticData * ioh = new IOHStaticData(path, IOHStaticData::Directory, true);


		PathMappingList::iterator it;
		for(it=node->children.begin();
		    it!=node->children.end();
			++it)
		{
			if(!((*it)->PathPattern == "*"))
			{
				linux::dirent64 de;
				de.d_ino = 0; //dummy value
				de.d_type = 0; //not provided on linux x86 32bit?  (GetUnixFileType(p);
				StringCbCopy(de.d_name, sizeof(de.d_name), (*it)->PathPattern);

				ioh->AddData(de);
			}
		}

		if(node->lister.Lister0)
		{
			DirEnt64List lst;

			switch(node->NumParams)
			{
			case 0:
				node->lister.Lister0(lst);
				break;
			case 1:
				node->lister.Lister1(lst, wildcards[0]);
				break;
			default:
				ktrace("Implement GenericStatic lister for params: %d\n", node->NumParams);
				return NULL;
			}

			while(!lst.empty())
			{
				ioh->AddData( *lst.begin() );
				lst.erase(lst.begin());
			}
		}

		return ioh;
	}
}

string FilesystemGenericStatic::GetPathSeperator()
{
	return "/";
}

bool FilesystemGenericStatic::IsSymbolicLink(string& strPath)
{
	//no sym links in static filesystems like procfs?
	return false;
}

string FilesystemGenericStatic::GetLinkDestination(string& strPath)
{
	//no sym links in static filesystems like procfs?
	return "";
}

bool FilesystemGenericStatic::IsRelativePath(string& strPath)
{
	return strPath[0]!='/';
}

const char * FilesystemGenericStatic::Name()
{
	return m_pFsName;
}


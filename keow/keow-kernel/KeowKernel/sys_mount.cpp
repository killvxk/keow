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

#include "includes.h"
#include "SysCalls.h"

#include "FilesystemKeow.h"
#include "FilesystemProc.h"
#include "FilesystemDev.h"

// eax is the syscall number
// ebx,ecx,edx,esi,edi,ebp are up to 6(max) parameters
// any more parameters and the caller just puts a struct pointer in one of these
// eax is the return value



/*
 * int  mount(const char *source, const char *target, const char *filesystemtype,
 *				unsigned long mountflags, const void *data);
 */
void SysCalls::sys_mount(CONTEXT& ctx)
{
	string source = MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx);
	string target = MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ecx);
	string filesystemtype = MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Edx);
	unsigned long mountflags = ctx.Esi;
	const void *data = (const void*)ctx.Edi;

	if(target.length()==0
	|| filesystemtype.length()==0)
	{
		ctx.Eax = -linux::EINVAL;
		return;
	}
	ktrace("mount request: '%s' on '%s' - type %s\n", source.c_str(),target.c_str(),filesystemtype.c_str());

	//the target must always be a directory
	Path p(false);
	p.SetUnixPath(target);

	DWORD attr = GetFileAttributes(p.GetWin32Path());
	if(attr==INVALID_FILE_ATTRIBUTES)
	{
		ctx.Eax = -linux::ENOTDIR;
		return;
	}
	if((attr&FILE_ATTRIBUTE_DIRECTORY)==0)
	{
		ctx.Eax = -linux::ENOTDIR;
		return;
	}


	//TODO: make these if's a lookup for plugins etc
	if(filesystemtype=="keow")
	{
		//expect the source to be a valid win32 directory path
		//and we also need it to end in a backslash if it is just a drive letter
		if(source.length()==0 || source[1]==':' && source[2]==0) {
			//just drive letter, need path:  eg. C: bad,  C:\ is ok
			ctx.Eax = -linux::ENOTDIR;
			return;
		}
		attr = GetFileAttributes(source);
		if(attr==INVALID_FILE_ATTRIBUTES)
		{
			ctx.Eax = -linux::ENOTDIR;
			return;
		}
		if((attr&FILE_ATTRIBUTE_DIRECTORY)==0)
		{
			ctx.Eax = -linux::ENOTDIR;
			return;
		}

		//copy of the mount data
		string MntData = MemoryHelper::ReadString(P->m_hProcess, (ADDR)data);

		//perform the mount

		Filesystem * pFS = new FilesystemKeow();
		MountPoint * pMP = MountPoint::Mount(Path(target), source, pFS, mountflags, (LPBYTE)MntData.c_str(), MntData.length());

		ctx.Eax = 0;
	}
	else
	if(filesystemtype=="proc")
	{
		Filesystem * pFS = new FilesystemProc();
		MountPoint * pMP = MountPoint::Mount(Path(target), source, pFS, mountflags, NULL, 0);

		ctx.Eax = 0;
	}
	else
	if(filesystemtype=="dev")
	{
		Filesystem * pFS = new FilesystemDev();
		MountPoint * pMP = MountPoint::Mount(Path(target), source, pFS, mountflags, NULL, 0);

		ctx.Eax = 0;
	}
	else
	{
		ctx.Eax = -linux::ENODEV; //filesystem type not supported
		return;
	}
}

/*****************************************************************************/

/*
 * int umount(const char *target)
 */
void SysCalls::sys_umount(CONTEXT& ctx)
{
	string target = MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx);

	if(target.length()==0)
	{
		ctx.Eax = -linux::EINVAL;
		return;
	}

	KernelTable::MountPointList::iterator it, itFound;
	itFound=g_pKernelTable->m_MountPoints.end(); 
	
	for(it=g_pKernelTable->m_MountPoints.begin();
	    it!=g_pKernelTable->m_MountPoints.end();
		++it)
	{
		MountPoint * mp = *it;

		if(mp->GetUnixMountPoint() == target)
		{
			itFound = it;
			//keep going - unmount last one found
		}
	}
	if(itFound!=g_pKernelTable->m_MountPoints.end())
	{
		ktrace("umount request: '%s'\n", target.c_str());
		g_pKernelTable->m_MountPoints.erase(itFound);
		ctx.Eax = 0;
		return;
	}

	//nothing found
	ctx.Eax = -linux::EINVAL;
}


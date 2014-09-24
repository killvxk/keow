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


// eax is the syscall number
// ebx,ecx,edx,esi,edi,ebp are up to 6(max) parameters
// any more parameters and the caller just puts a struct pointer in one of these
// eax is the return value

void SysCalls::sys_getuid(CONTEXT& ctx)
{
	ctx.Eax = P->m_uid;
}

void SysCalls::sys_geteuid(CONTEXT& ctx)
{
	ctx.Eax = P->m_euid;
}

void SysCalls::sys_getgid(CONTEXT& ctx)
{
	ctx.Eax = P->m_gid;
}

void SysCalls::sys_getegid(CONTEXT& ctx)
{
	ctx.Eax = P->m_egid;
}


void SysCalls::sys_getuid32(CONTEXT& ctx)
{
	ctx.Eax = P->m_uid;
}

void SysCalls::sys_geteuid32(CONTEXT& ctx)
{
	ctx.Eax = P->m_euid;
}

void SysCalls::sys_getgid32(CONTEXT& ctx)
{
	ctx.Eax = P->m_gid;
}

void SysCalls::sys_getegid32(CONTEXT& ctx)
{
	ctx.Eax = P->m_egid;
}

/*****************************************************************************/

/*
 * int setreuid(int ruid, int euid)
 */
void SysCalls::sys_setreuid(CONTEXT& ctx)
{
	int ruid = ctx.Ebx;
	int euid = ctx.Ecx;

	ctx.Eax = -linux::EPERM;

	if(ruid != -1)
	{
		//only root can set real uid
		if(P->m_uid == 0
		|| P->m_euid == 0)
		{
			P->m_uid = ruid;
			ctx.Eax = 0;
		}
	}

	if(euid != -1)
	{
		//root can set to anything
		//users can set to any existing id
		if(P->m_uid == 0
		|| P->m_euid == 0
		|| euid == P->m_uid
		|| euid == P->m_euid
		|| euid == P->m_saved_uid )
		{
			P->m_euid = euid;
			ctx.Eax = 0;
		}
	}

}

/*
 * int setregid(int rgid, int egid)
 */
void SysCalls::sys_setregid(CONTEXT& ctx)
{
	int rgid = ctx.Ebx;
	int egid = ctx.Ecx;

	ctx.Eax = -linux::EPERM;

	if(rgid != -1)
	{
		//only root can set real gid
		if(P->m_uid == 0
		|| P->m_euid == 0)
		{
			P->m_gid = rgid;
			ctx.Eax = 0;
		}
	}

	if(egid != -1)
	{
		//root can set to anything
		//users can set to any existing id
		if(P->m_uid == 0
		|| P->m_euid == 0
		|| egid == P->m_gid
		|| egid == P->m_egid
		|| egid == P->m_saved_gid )
		{
			P->m_egid = egid;
			ctx.Eax = 0;
		}
	}

}

void SysCalls::sys_setreuid32(CONTEXT& ctx)
{
	sys_setreuid(ctx);
}

void SysCalls::sys_setregid32(CONTEXT& ctx)
{
	sys_setregid(ctx);
}


/*
 * int chmod(const char *path, mode_t mode);
 */
void SysCalls::sys_chmod(CONTEXT& ctx)
{
	ktrace("IMPLEMENT proper permissions changes\n");

	//pretent we worked
	ctx.Eax = 0;
}

/*
 * int fchmod(int fildes, mode_t mode);
 */
void SysCalls::sys_fchmod(CONTEXT& ctx)
{
	ktrace("IMPLEMENT proper permissions changes\n");

	//pretent we worked
	ctx.Eax = 0;
}

void SysCalls::sys_chown(CONTEXT& ctx)
{
	ktrace("IMPLEMENT proper permissions changes\n");

	//pretent we worked
	ctx.Eax = 0;
}

void SysCalls::sys_lchown(CONTEXT& ctx)
{
	ktrace("IMPLEMENT proper permissions changes\n");

	//pretent we worked
	ctx.Eax = 0;
}

void SysCalls::sys_fchown(CONTEXT& ctx)
{
	ktrace("IMPLEMENT proper permissions changes\n");

	//pretent we worked
	ctx.Eax = 0;
}

void SysCalls::sys_lchown32(CONTEXT& ctx)
{
	ktrace("IMPLEMENT proper permissions changes\n");

	//pretent we worked
	ctx.Eax = 0;
}

void SysCalls::sys_fchown32(CONTEXT& ctx)
{
	ktrace("IMPLEMENT proper permissions changes\n");

	//pretent we worked
	ctx.Eax = 0;
}

void SysCalls::sys_chown32(CONTEXT& ctx)
{
	ktrace("IMPLEMENT proper permissions changes\n");

	//pretent we worked
	ctx.Eax = 0;
}


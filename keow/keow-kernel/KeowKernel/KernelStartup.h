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

// KernelStartup.h: interface for the KernelStartup class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_KERNELSTARTUP_H__B2AA20E4_FD73_4EA4_8C87_E3F083105A7D__INCLUDED_)
#define AFX_KERNELSTARTUP_H__B2AA20E4_FD73_4EA4_8C87_E3F083105A7D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class KernelStartup  
{
public:
	static void ProcessCommandLine(LPSTR lpCmdLine);
	static void ValidateKernelTraps();
	static void AutoMountDrives();

	static char * GetInitProgram()  {
		return s_InitArguments[0];
	}
	static char ** GetInitArguments()  {
		return s_InitArguments;
	}

protected:
	static const int s_InitMaxArgs;
	static int s_InitCurrentArgs;
	static char ** s_InitArguments;
	static char * s_pszAutoMount;

	typedef void (*ARG_HANDLER)(const char *);
	struct HandlerRec {
		const char * arg_name;
		ARG_HANDLER handler;
	};
	static HandlerRec s_ArgumentHandlers[];

	static void arg_root(const char *);
	static void arg_init(const char *);
	static void arg_debug(const char *);
	static void arg_automount(const char *);
	static void arg_log(const char *);
};

#endif // !defined(AFX_KERNELSTARTUP_H__B2AA20E4_FD73_4EA4_8C87_E3F083105A7D__INCLUDED_)

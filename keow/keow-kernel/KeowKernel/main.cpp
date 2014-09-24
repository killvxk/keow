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

// Main


#include "includes.h"
#include "KernelStartup.h"


//single copy of the kernel data
KernelTable * g_pKernelTable;

//Thread local stuff for current thread state in kernel handler
//This allows all the various kernal functions to access the current (P)rocess and (T)hread
//static const int KTRACE_BUFFER_SIZE = 32000;
_declspec(thread) char * g_pTraceBuffer;
_declspec(thread) Process * P;
_declspec(thread) ThreadInfo * T;

Process * g_pInit;

static char * InitialEnv[] = {
	"OLDPWD=/",
	"HOME=/",
	"TERM=ansi80x25", //freebsd console (25-line ansi mode) -- easier to implement than the linux console -- implemented in ioh_console.cpp
	"PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin",
	"PWD=/",
//	"LD_DEBUG=libs",
//	"LD_SHOW_AUXV=y",
//	"LD_BIND_NOW=y",
//	"LD_TRACE_LOADED_OBJECTS=y",
//	"LD_VERBOSE=y",
	NULL
};


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	g_pTraceBuffer = new char [KTRACE_BUFFER_SIZE];
	P = NULL;

	ktrace("Kernel Booting\n");

	g_pKernelTable = new KernelTable();


	// A file time is a 64-bit value that represents the number of 100-nanosecond intervals 
	// that have elapsed since 12:00 A.M. January 1, 1601 (UTC).
	// A unix time_t is time since the Epoch (00:00:00 UTC, January 1, 1970), in seconds.
	// Need a base time for calculations
	ktrace("calc 1-jan-1970\n");
	SYSTEMTIME st;
	FILETIME ft, ft2;
	st.wDayOfWeek = 0;
	st.wDay = 1;
	st.wMonth = 1;
	st.wYear = 1970;
	st.wHour = 0;
	st.wMinute = 0;
	st.wSecond = 0;
	st.wMilliseconds = 0;
	SystemTimeToFileTime(&st, &ft);
	st.wSecond = 1;
	SystemTimeToFileTime(&st, &ft2);
	Time1Jan1970.LowPart = ft.dwLowDateTime;
	Time1Jan1970.HighPart = ft.dwHighDateTime;
	ktrace("calculated win32 1-jan-1970 (for unix epoch)\n");

	KernelStartup::ValidateKernelTraps();

	CoInitialize(NULL); //Ex(NULL, COINIT_MULTITHREADED);

	KernelStartup::ProcessCommandLine(lpCmdLine);
	if(g_pKernelTable->m_FilesystemRoot.length() == 0)
	{
		ktrace("No root= specified\n");
		halt();
	}


	//AutoMount windows drive letters
	KernelStartup::AutoMountDrives();
	
	
	//Load the initial executable
	ktrace("Loading 'init'");
	Path InitPath;
	char ** InitialArgs;
	InitPath.SetUnixPath( KernelStartup::GetInitProgram() );
	InitialArgs = KernelStartup::GetInitArguments();
	Process * proc = Process::StartInit(1, InitPath, InitialArgs, InitialEnv);
	if(proc == 0)
	{
		ktrace("Failed to load 'init'\n");
		halt();
	}
	g_pInit=proc;

	ktrace("'init' launched\n");

	//wait for it to exit
	MsgWaitForMultipleObjects(1, &proc->m_hProcess, FALSE, INFINITE, NULL);
	ktrace("'init' appears to have terminated, ");

	ktrace("exit code 0x%lx (%ld)\n", proc->m_dwExitCode,proc->m_dwExitCode);


	halt();
	return -1; //never get here
}

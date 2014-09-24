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

// Stub Main

#include "includes.h"


//global
extern SysCallDll::RemoteAddrInfo AddrInfo;

static void LoadAddressInfo()
{
	memset(&AddrInfo, 0, sizeof(AddrInfo));

#define SET_ADDR(func) AddrInfo.##func = (LPVOID)(SysCallDll::##func )

	SET_ADDR(VirtualAlloc);
	SET_ADDR(VirtualFree);
	SET_ADDR(GetLastError);
	SET_ADDR(CloseHandle);
	SET_ADDR(SetFilePointer);
	SET_ADDR(GetFilePointer);
	SET_ADDR(SetEndOfFile);
	SET_ADDR(ZeroMem);
	SET_ADDR(exit);
	SET_ADDR(CreateFileMapping);
	SET_ADDR(MapViewOfFileEx);
	SET_ADDR(UnmapViewOfFile);
	SET_ADDR(WriteFile);
	SET_ADDR(ReadFile);
	SET_ADDR(PeekAvailablePipe);

#undef SET_ADDR
}


static char g_TraceBuffer[32768];

void ktrace(const char * format, ...)
{
	va_list va;
	va_start(va, format);

	wvsprintf(g_TraceBuffer, format, va);

	OutputDebugString(g_TraceBuffer);
}


#if 1
void DumpMemory(BYTE * addr, DWORD len)
{
	const int BYTES_PER_LINE = 8;
	char hexbuf[5 * BYTES_PER_LINE + 1]; // "0x00 "... + null
	char charbuf[BYTES_PER_LINE + 1];    // "xxxxxxxx" + null
	int x;
	BYTE b;

	memset(charbuf, 0, sizeof(charbuf));

	ktrace("memory dump @ 0x%08lx, len %d\n", addr,len);
	x=0;
	for(DWORD i=0; i<len; ++i)
	{
		b = addr[x];
		wsprintf(&hexbuf[x*5], "0x%02x ", b);
		charbuf[x] = (isalnum(b)||ispunct(b)) ? b : '.';

		++x;
		if(x>=BYTES_PER_LINE)
		{
			ktrace("  0x%08lx: %s %s\n", addr, hexbuf, charbuf);
			addr+=BYTES_PER_LINE;
			x=0;
			memset(charbuf, 0, sizeof(charbuf));
		}
	}
	if(x>0)
		ktrace("  0x%08lx: %s %s\n", addr, hexbuf, charbuf);
}
#endif


BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason, LPVOID p)
{
	switch(dwReason)
	{
	case DLL_PROCESS_ATTACH:
		//Set the info about the win32 side.
		//The stub provides this info in Eax prior to causing an initial breakpoint 
		//to send it to the kernel
		//(see Process::ConvertProcessToKeow()

		OutputDebugString("SysCallDll loading\n");
		LoadAddressInfo();

		//pass control to the kernel to re-develop us into a keow process
		OutputDebugString("SysCallDll transfering to kernel\n");
		__asm {
			lea ebx, AddrInfo

			//set single-step mode to tell kernel we are ready
			pushfd
			pop eax
			or eax, 0x100   //trap bit
			push eax
			popfd
			//now we are single-stepping

			nop
		}

		//should never get here - it's bad
		ExitProcess(-11);
		break;

	} //switch

	return TRUE;
}

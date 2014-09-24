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

/*
 * This include file defines the functions that are supplied in the
 * keow user syscall dll.
 * They allow some system calls to be run from the user/stub process directly.
 *
 * The kernel implementes these functions as calls to InvokeStubSysCall
 * and the user/stub implements them as the actual calls
 */

#if !defined(AFX_SYSCALLDLL_H__065A3BC3_71C3_4302_8E39_297E193A46AF__INCLUDED_)
#define AFX_SYSCALLDLL_H__065A3BC3_71C3_4302_8E39_297E193A46AF__INCLUDED_

//This file needs to not reference the other kernel structures
//because it is included in the user process
class Process; //exception to the rule, just to allow a pointer to the associated process on the kernel implementation

//We need the _stdcall convention to allow all functions to clean up the stack
//and not rely on the caller to do it.
//This means the kernel can inject a call (&stack params) into the user process
//and not need to inject any stack cleanup code.

//On both platforms (linux/win32) the function return value is in EAX.
//So all functions return a DWORD.
//The functions try to return the appropriate Unix error code
//

class SysCallDll
{
public:
	//functionality provided

#define SC static DWORD __stdcall

	SC VirtualAlloc(LPVOID lpAddress, DWORD dwSize, DWORD flAllocationType, DWORD flProtect);
	SC VirtualFree(LPVOID lpAddress, DWORD dwSize, DWORD dwFreeType);
	SC GetLastError();
	SC CloseHandle(HANDLE h);
	SC SetFilePointer(HANDLE h, DWORD PosLo, DWORD PosHi, DWORD from);
	SC GetFilePointer(HANDLE h);
	SC SetEndOfFile(HANDLE h);
	SC ZeroMem(void *p, DWORD len);
	SC exit(UINT exitcode);
	SC CreateFileMapping(HANDLE hFile, DWORD Prot, DWORD sizeHi, DWORD sizeLo);
	SC MapViewOfFileEx(HANDLE hMap, DWORD Prot, DWORD offsetHi, DWORD offsetLo, DWORD len, void* BaseAddr);
	SC UnmapViewOfFile(void* BaseAddr);
	SC WriteFile(HANDLE h, LPVOID buf, DWORD len);
	SC ReadFile(HANDLE h, LPVOID buf, DWORD len);
	SC PeekAvailablePipe(HANDLE h);

#undef SC

	//corresponding addresses
	struct RemoteAddrInfo {
		LPVOID VirtualAlloc;
		LPVOID VirtualFree;
		LPVOID GetLastError;
		LPVOID CloseHandle;
		LPVOID SetFilePointer;
		LPVOID GetFilePointer;
		LPVOID SetEndOfFile;
		LPVOID ZeroMem;
		LPVOID exit;
		LPVOID CreateFileMapping;
		LPVOID MapViewOfFileEx;
		LPVOID UnmapViewOfFile;
		LPVOID WriteFile;
		LPVOID ReadFile;
		LPVOID PeekAvailablePipe;
	};
};


#endif

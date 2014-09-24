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

// Kernel side implementation of the SysCallDll class.
//
////////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "SysCallDll.h"


////////////////////////////////////////////////////////////////////////

DWORD _stdcall SysCallDll::VirtualAlloc(LPVOID lpAddress, DWORD dwSize, DWORD flAllocationType, DWORD flProtect)
{
	DWORD stack[] = {
		(DWORD)lpAddress,
		dwSize, 
		flAllocationType,
		flProtect
	};
	return P->InjectFunctionCall(P->SysCallAddr.VirtualAlloc, &stack, sizeof(stack));
}

DWORD _stdcall SysCallDll::VirtualFree(LPVOID lpAddress, DWORD dwSize, DWORD dwFreeType)
{
	DWORD stack[] = {
		(DWORD)lpAddress, 
		dwSize, 
		dwFreeType
	};
	return P->InjectFunctionCall(P->SysCallAddr.VirtualFree, &stack, sizeof(stack));
}

/* left here as an example of how to call a function with a large structure/buffer to deal with
DWORD _stdcall SysCallDll::VirtualQuery(LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength)
{
	struct {
		DWORD lpAddress;
		DWORD lpBuffer;
		DWORD dwLength;
		MEMORY_BASIC_INFORMATION MemBuf;
	} stack;
	stack.lpAddress = (DWORD)lpAddress;
	stack.dwLength = dwLength;

	Process::InjectionInfo info[] = {
		{ 1, (DWORD)&stack.MemBuf-(DWORD)&stack }
	};

	DWORD ret = P->InjectFunctionCall(P->SysCallAddr.VirtualQuery, &stack, sizeof(stack), info, 1);
	memcpy(lpBuffer, &stack.MemBuf, sizeof(stack.MemBuf));
	return ret;
}*/


DWORD _stdcall SysCallDll::GetLastError()
{
	return P->InjectFunctionCall(P->SysCallAddr.GetLastError, NULL, 0);
}


DWORD _stdcall SysCallDll::CloseHandle(HANDLE h)
{
	DWORD stack[] = {
		(DWORD)h
	};
	return P->InjectFunctionCall(P->SysCallAddr.CloseHandle, &stack, sizeof(stack));
}


DWORD _stdcall SysCallDll::SetFilePointer(HANDLE h, DWORD PosLo, DWORD PosHi, DWORD from)
{
	DWORD stack[] = {
		(DWORD)h,
		(DWORD)PosLo,
		(DWORD)PosHi,
		(DWORD)from,
	};
	return P->InjectFunctionCall(P->SysCallAddr.SetFilePointer, &stack, sizeof(stack));
}

DWORD _stdcall SysCallDll::GetFilePointer(HANDLE h)
{
	DWORD stack[] = {
		(DWORD)h
	};
	return P->InjectFunctionCall(P->SysCallAddr.GetFilePointer, &stack, sizeof(stack));
}


DWORD _stdcall SysCallDll::SetEndOfFile(HANDLE h)
{
	DWORD stack[] = {
		(DWORD)h
	};
	return P->InjectFunctionCall(P->SysCallAddr.SetEndOfFile, &stack, sizeof(stack));
}


DWORD _stdcall SysCallDll::ZeroMem(void *p, DWORD len)
{
	DWORD stack[] = {
		(DWORD)p,
		len
	};
	return P->InjectFunctionCall(P->SysCallAddr.ZeroMem, &stack, sizeof(stack));
}


DWORD _stdcall SysCallDll::CreateFileMapping(HANDLE hFile, DWORD Prot, DWORD sizeHi, DWORD sizeLo)
{
	DWORD stack[] = {
		(DWORD)hFile,
		Prot,
		sizeHi,
		sizeLo
	};
	return P->InjectFunctionCall(P->SysCallAddr.CreateFileMapping, &stack, sizeof(stack));
}

DWORD _stdcall SysCallDll::MapViewOfFileEx(HANDLE hMap, DWORD Prot, DWORD offsetHi, DWORD offsetLo, DWORD len, void* BaseAddr)
{
	DWORD stack[] = {
		(DWORD)hMap,
		Prot,
		offsetHi,
		offsetLo,
		len,
		(DWORD)BaseAddr
	};
	return P->InjectFunctionCall(P->SysCallAddr.MapViewOfFileEx, &stack, sizeof(stack));
}

DWORD _stdcall SysCallDll::UnmapViewOfFile(void* BaseAddr)
{
	DWORD stack[] = {
		(DWORD)BaseAddr
	};
	return P->InjectFunctionCall(P->SysCallAddr.UnmapViewOfFile, &stack, sizeof(stack));
}


DWORD _stdcall SysCallDll::exit(UINT exitcode)
{
	return P->InjectFunctionCall(P->SysCallAddr.exit, &exitcode, sizeof(UINT));
}


DWORD _stdcall SysCallDll::WriteFile(HANDLE h, LPVOID buf, DWORD len)
{
	DWORD stack[] = {
		(DWORD)h,
		(DWORD)buf,
		len
	};
	return P->InjectFunctionCall(P->SysCallAddr.WriteFile, &stack, sizeof(stack));
}

DWORD _stdcall SysCallDll::ReadFile(HANDLE h, LPVOID buf, DWORD len)
{
	DWORD stack[] = {
		(DWORD)h,
		(DWORD)buf,
		len
	};
	return P->InjectFunctionCall(P->SysCallAddr.ReadFile, &stack, sizeof(stack));
}

DWORD _stdcall SysCallDll::PeekAvailablePipe(HANDLE h)
{
	DWORD stack[] = {
		(DWORD)h
	};
	return P->InjectFunctionCall(P->SysCallAddr.PeekAvailablePipe, &stack, sizeof(stack));
}


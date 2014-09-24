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

// LegacyWindows.cpp: implementation of the LegacyWindows class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "LegacyWindows.h"

//#define KEOW_FORCE_WIN95_VERSIONS
#undef KEOW_FORCE_WIN95_VERSIONS

//////////////////////////////////////////////////////////////////////

BOOL LegacyWindows::IsProcessorFeaturePresent(DWORD dwFeature)
{
	HMODULE hlib = GetModuleHandle("KERNEL32");
	FARPROC fp = GetProcAddress(hlib, "IsProcessorFeaturePresent");
#ifdef KEOW_FORCE_WIN95_VERSIONS
	fp=0;
#endif

	if(fp)
	{
		//can use the real function

		BOOL (CALLBACK *RealIsProcessorFeaturePresent)
				 (DWORD);
        *(FARPROC *)&RealIsProcessorFeaturePresent = fp;
		return RealIsProcessorFeaturePresent(dwFeature);
	}
	else
	{
		//emulate it

		switch(dwFeature)
		{
		case PF_FLOATING_POINT_EMULATED:
			return FALSE;
		default:
			return FALSE;
		}
	}
}


BOOL LegacyWindows::GetFileAttributesEx(LPCTSTR lpFileName, GET_FILEEX_INFO_LEVELS fInfoLevelId, LPVOID lpFileInformation)
{
	HMODULE hlib = GetModuleHandle("KERNEL32");
	FARPROC fp = GetProcAddress(hlib, "GetFileAttributesExA");
#ifdef KEOW_FORCE_WIN95_VERSIONS
	fp=0;
#endif

	if(fp) 
	{
		//can use the real function

		BOOL (CALLBACK *RealGetFileAttributesEx)
				 (LPCTSTR, GET_FILEEX_INFO_LEVELS, LPVOID);
        *(FARPROC *)&RealGetFileAttributesEx = fp;
		if(!RealGetFileAttributesEx(lpFileName, fInfoLevelId, lpFileInformation))
			return FALSE;
	}
	else
	{
		//emulate it

		if(fInfoLevelId != GetFileExInfoStandard)
			return FALSE; //can't do it

		DWORD FileAddr = GetFileAttributes(lpFileName);
		if(FileAddr == INVALID_FILE_ATTRIBUTES)
			return FALSE;

        HANDLE hfind;
        WIN32_FIND_DATA wfd;
		string find = lpFileName;
		if(FileAddr & FILE_ATTRIBUTE_DIRECTORY)
			find += "\\*.*";
        hfind = FindFirstFile(find, &wfd);
        if(hfind == INVALID_HANDLE_VALUE)
			return FALSE;

        FindClose(hfind);

		WIN32_FILE_ATTRIBUTE_DATA * pFA = (WIN32_FILE_ATTRIBUTE_DATA *)lpFileInformation;
        pFA->dwFileAttributes = wfd.dwFileAttributes;
        pFA->ftCreationTime   = wfd.ftCreationTime;
        pFA->ftLastAccessTime = wfd.ftLastAccessTime;
        pFA->ftLastWriteTime  = wfd.ftLastWriteTime;
        pFA->nFileSizeHigh    = wfd.nFileSizeHigh;
        pFA->nFileSizeLow     = wfd.nFileSizeLow;
	}

	return TRUE;
}


LPVOID LegacyWindows::VirtualAllocEx(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect)
{
	if(hProcess == GetCurrentProcess())
		return VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);

#ifdef _DEBUG
	if(hProcess != P->m_hProcess)
		DebugBreak();
#endif

	HMODULE hlib = GetModuleHandle("KERNEL32");
	FARPROC fp = GetProcAddress(hlib, "VirtualAllocEx");
#ifdef KEOW_FORCE_WIN95_VERSIONS
	fp=0;
#endif

	if(fp) 
	{
		//can use the real function

		LPVOID (CALLBACK *RealVirtualAllocEx)
				 (HANDLE, LPVOID, SIZE_T, DWORD, DWORD);
        *(FARPROC *)&RealVirtualAllocEx = fp;
		return RealVirtualAllocEx(hProcess, lpAddress, dwSize, flAllocationType, flProtect);
	}
	else
	{
		//get stub to do it

		return (LPVOID)SysCallDll::VirtualAlloc(lpAddress, dwSize, flAllocationType, flProtect);
	}
}

BOOL LegacyWindows::VirtualFreeEx(HANDLE hProcess, LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType)
{
	if(hProcess == GetCurrentProcess())
		return VirtualFree(lpAddress, dwSize, dwFreeType);

#ifdef _DEBUG
	if(hProcess != P->m_hProcess)
		DebugBreak();
#endif

	HMODULE hlib = GetModuleHandle("KERNEL32");
	FARPROC fp = GetProcAddress(hlib, "VirtualFreeEx");
#ifdef KEOW_FORCE_WIN95_VERSIONS
	fp=0;
#endif

	if(fp) 
	{
		//can use the real function

		BOOL (CALLBACK *RealVirtualFreeEx)
				 (HANDLE, LPVOID, SIZE_T, DWORD);
        *(FARPROC *)&RealVirtualFreeEx = fp;
		return RealVirtualFreeEx(hProcess, lpAddress, dwSize, dwFreeType);
	}
	else
	{
		//get stub to do it

		return SysCallDll::VirtualFree(lpAddress, dwSize, dwFreeType);
	}
}

DWORD LegacyWindows::VirtualQueryEx(HANDLE hProcess, LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer, SIZE_T dwLength)
{
	//Windows 95 does have VirtualQueryEx
	//we just have this routine here for consisency
	//(all routines using VirtualXXX functions will be using this class)

	return ::VirtualQueryEx(hProcess, lpAddress, lpBuffer, dwLength);
}


BOOL LegacyWindows::CreateHardLink(LPCSTR lpNewFile, LPCSTR lpOldFile)
{
	HMODULE hlib = GetModuleHandle("KERNEL32");
	FARPROC fp = GetProcAddress(hlib, "CreateHardLinkA");
#ifdef KEOW_FORCE_WIN95_VERSIONS
	fp=0;
#endif

	if(fp)
	{
		//can use the real function

		BOOL (CALLBACK *RealCreateHardLinkA)
				 (LPCTSTR, LPCTSTR, LPSECURITY_ATTRIBUTES);
        *(FARPROC *)&RealCreateHardLinkA = fp;
		return RealCreateHardLinkA(lpNewFile, lpOldFile, NULL);
	}
	else
	{
		//can't do anything like this on win9x

		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}
}

NTSTATUS LegacyWindows::NtQueryInformationProcess(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength)
{
	HMODULE hlib = GetModuleHandle("NTDLL");
	FARPROC fp = GetProcAddress(hlib, "NtQueryInformationProcess");
#ifdef KEOW_FORCE_WIN95_VERSIONS
	fp=0;
#endif

	if(fp)
	{
		//can use the real function

		NTSTATUS (CALLBACK *NtQueryInformationProcess)
				 (HANDLE, PROCESSINFOCLASS, PVOID, ULONG, PULONG);
        *(FARPROC *)&NtQueryInformationProcess = fp;
		return NtQueryInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength, ReturnLength);
	}
	else
	{
		//can't do anything like this on win9x

		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}
}

NTSTATUS LegacyWindows::NtSetInformationProcess(HANDLE ProcessHandle, PROCESSINFOCLASS ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength)
{
	HMODULE hlib = GetModuleHandle("NTDLL");
	FARPROC fp = GetProcAddress(hlib, "NtSetInformationProcess");
#ifdef KEOW_FORCE_WIN95_VERSIONS
	fp=0;
#endif

	if(fp)
	{
		//can use the real function

		NTSTATUS (CALLBACK *NtSetInformationProcess)
				 (HANDLE, PROCESSINFOCLASS, PVOID, ULONG);
        *(FARPROC *)&NtSetInformationProcess = fp;
		return NtSetInformationProcess(ProcessHandle, ProcessInformationClass, ProcessInformation, ProcessInformationLength);
	}
	else
	{
		//can't do anything like this on win9x?

		SetLastError(ERROR_INVALID_FUNCTION);
		return FALSE;
	}
}

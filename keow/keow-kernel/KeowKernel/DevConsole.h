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

// DevConsole.h: interface for the DevConsole class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DEVCONSOLE_H__8E978E10_AA78_4039_9F10_847E0D757C27__INCLUDED_)
#define AFX_DEVCONSOLE_H__8E978E10_AA78_4039_9F10_847E0D757C27__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class DevConsole : public Device
{
public:
	friend class IOHNtConsole;

	DevConsole(int tty);
	virtual ~DevConsole();

	DWORD ioctl(DWORD request, DWORD data);

private:
	int m_tty;
	HANDLE m_hConsoleWrite, m_hConsoleRead;
	HANDLE m_hIoctlWrite, m_hIoctlRead;
	CRITICAL_SECTION m_csIoctl;
	linux::pid_t m_ProcessGroup;
	linux::termios m_TermIOs;

	//kernel interception of reading the console
	HANDLE m_hRealConsoleRead;
	HANDLE m_hConsoleReadBufferWrite; //kernel reads real console and writes to this pipe (which m_hConsoleRead reads from)

	HANDLE m_hHandlerThread;
	DWORD m_dwHandlerThreadId;

private:
	//console IO handler thread
	static DWORD WINAPI ConsoleHandlerEntry(LPVOID param);
	static const DWORD KERNEL_CONSOLE_HANDLER_STACK_SIZE;

	DWORD ConsoleHandlerMain();
};

#endif // !defined(AFX_DEVCONSOLE_H__8E978E10_AA78_4039_9F10_847E0D757C27__INCLUDED_)

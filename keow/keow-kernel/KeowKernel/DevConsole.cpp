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

// DevConsole.cpp: implementation of the DevConsole class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "DevConsole.h"


const DWORD DevConsole::KERNEL_CONSOLE_HANDLER_STACK_SIZE = 2048; //hardly need anything

#define CTRL(ch)  (ch-'A'+1)

#define CR 0x0d
#define NL 0x0a //NewLine/LineFeed
#define LF 0x0a //NewLine/LineFeed
#define FF 0x0c //formfeed
#define VT 0x0b //vertical tab
#define TAB 0x09
#define ESC 0x1b
#define BACKSPACE 0x08
#define SO 0x0e //active G1 character set
#define SI 0x0f //active G0 character set
#define CAN 0x18
#define SUB 0x1a 
#define DEL 0x7f 

//////////////////////////////////////////////////////////////////////

DevConsole::DevConsole(int tty)
: Device("tty", 0, 0)
{
	ktrace("creating new console, tty%d\n", tty);

	m_tty = tty;
	m_ProcessGroup = 0;
	m_hConsoleRead = m_hConsoleWrite = NULL;

	//default stty info
	m_TermIOs.c_iflag = linux::IGNBRK|linux::IGNPAR|linux::IXOFF|linux::ISIG;
	m_TermIOs.c_oflag = 0;
	m_TermIOs.c_cflag = linux::CS8;
	m_TermIOs.c_lflag = linux::ECHO;
	m_TermIOs.c_line = linux::N_TTY;
	memset(m_TermIOs.c_cc, 0, sizeof(m_TermIOs.c_cc));

/*      intr=^C         quit=^\         erase=del       kill=^U
        eof=^D          vtime=\0        vmin=\1         sxtc=\0
        start=^Q        stop=^S         susp=^Z         eol=\0
        reprint=^R      discard=^U      werase=^W       lnext=^V
        eol2=\0
*/
#define INIT_C_CC "\003\034\177\025\004\0\1\0\021\023\032\0\022\017\027\026\0"
	StringCbCopy((char*)m_TermIOs.c_cc, sizeof(m_TermIOs.c_cc), INIT_C_CC); //default special chars from kernel files


	InitializeCriticalSection(&m_csIoctl);

	//need a console process to do the actual terminal stuff

	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	//make stdin, stdout talk with us
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;

	memset(&si,0,sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES;
	CreatePipe( &si.hStdInput, &m_hConsoleWrite, &sa, 0);
	CreatePipe( &m_hRealConsoleRead, &si.hStdOutput, &sa, 0);
	//kernel read interception
	CreatePipe( &m_hConsoleRead, &m_hConsoleReadBufferWrite, &sa, 0);

	HANDLE hIoctlIn, hIoctlOut;
	CreatePipe( &hIoctlOut, &m_hIoctlWrite, &sa, 0);
	CreatePipe( &m_hIoctlRead, &hIoctlIn, &sa, 0);

	string cmdline = "KeowConsole.exe ";
	cmdline += string::format("%ld ", GetCurrentProcessId());
	cmdline += string::format("%ld ", si.hStdInput);
	cmdline += string::format("%ld ", si.hStdOutput);
	cmdline += string::format("%ld ", hIoctlOut);
	cmdline += string::format("%ld ", hIoctlIn);
	cmdline += " \"Keow Console\"";

	CreateProcess(NULL, cmdline.GetBuffer(cmdline.length()), 0, 0, TRUE, 0, 0, 0, &si, &pi);
	cmdline.ReleaseBuffer();

	//we need to handle io thru the kernel (not just to the processes directly)
	//this is so the kernel can see and handle Ctrl-C before a process is ready to read
	m_hHandlerThread = CreateThread(NULL, KERNEL_CONSOLE_HANDLER_STACK_SIZE, ConsoleHandlerEntry, this, 0, &m_dwHandlerThreadId);

	ktrace("console tty%d created\n", tty);
}

DevConsole::~DevConsole()
{
	m_hHandlerThread = CreateThread(NULL, KERNEL_CONSOLE_HANDLER_STACK_SIZE, ConsoleHandlerEntry, this, 0, &m_dwHandlerThreadId);

	CloseHandle(m_hConsoleRead);
	CloseHandle(m_hConsoleWrite);
	CloseHandle(m_hIoctlRead);
	CloseHandle(m_hIoctlWrite);
	DeleteCriticalSection(&m_csIoctl);
}


//Kernel interception of read events
/*static*/ DWORD WINAPI DevConsole::ConsoleHandlerEntry(LPVOID param)
{
	DevConsole * p = (DevConsole*)param;
	return p->ConsoleHandlerMain();
}

DWORD DevConsole::ConsoleHandlerMain()
{
	char buf[256];
	int max_read = sizeof(buf)/2; //allow for chars to be doubled up (eg CR -> CR+NL)
	DWORD dwRead, dwWritten;

	for(;;)
	{
		if(!ReadFile(m_hRealConsoleRead, buf, max_read, &dwRead, NULL))
		{
			//eof?
#ifdef _DEBUG
			DebugBreak();
#endif
			return 0;
		}

		//handle any special characters (eg Ctrl-C)
		for(DWORD i=0; i<dwRead; ++i)
		{
			char *p = &buf[i];

			//Input settings: c_iflag
			//#define IGNBRK	0000001
			//#define BRKINT	0000002
			//#define IGNPAR	0000004
			//#define PARMRK	0000010
			//#define INPCK	0000020
			//#define ISTRIP	0000040
			//#define INLCR	0000100
			//#define IGNCR	0000200
			//#define ICRNL	0000400
			//#define IUCLC	0001000
			//#define IXON	0002000
			//#define IXANY	0004000
			//#define IXOFF	0010000
			//#define IMAXBEL	0020000
			if((m_TermIOs.c_iflag & linux::ISTRIP))
				*p &= 0x3F; //make 7-bit

			if((m_TermIOs.c_iflag & linux::IGNCR) && *p==CR)
			{
				//discard from buffer
				for(DWORD j=i+1; j<dwRead; ++j)
					buf[j-1]=buf[j];
				--dwRead;
			}

			if((m_TermIOs.c_iflag & linux::INLCR) && *p==NL)
			{
				//insert char
				for(DWORD j=dwRead-1; j>i; --j)
					buf[j]=buf[j-1];
				++i;
				buf[i] = CR;
				++dwRead;
			}
			else
			if((m_TermIOs.c_iflag & linux::ICRNL) && *p==CR)
			{
				for(DWORD j=dwRead-1; j>i; --j)
					buf[j]=buf[j-1];
				++i;
				buf[i] = NL;
				++dwRead;
			}

			if((m_TermIOs.c_iflag & linux::IUCLC))
				*p = tolower(*p);


			//Local settings: c_lflag;
			//#define ISIG	0000001
			//#define ICANON	0000002
			//#define XCASE	0000004
			//#define ECHO	0000010
			//#define ECHOE	0000020
			//#define ECHOK	0000040
			//#define ECHONL	0000100
			//#define NOFLSH	0000200
			//#define TOSTOP	0000400
			//#define ECHOCTL	0001000
			//#define ECHOPRT	0002000
			//#define ECHOKE	0004000
			//#define FLUSHO	0010000
			//#define PENDIN	0040000
			//#define IEXTEN	0100000
			bool bSigSent = false;
			if(m_TermIOs.c_lflag & linux::ISIG)
			{
				if(*p==m_TermIOs.c_cc[linux::VINTR])
				{
					//send to all foreground processes in the process group on this terminal
					KernelTable::ProcessList::iterator it;
					for(it=g_pKernelTable->m_Processes.begin();
						it!=g_pKernelTable->m_Processes.end();
						++it)
					{
						Process * p2 = *it;
						if(p2->m_ProcessGroupPID == m_ProcessGroup
						&& p2->m_bIsInForeground)
						{
							p2->SendSignal(linux::SIGINT);
							bSigSent = true;
						}
					}
				}
				//others?
				//VQUIT
				//VERASE
				//VKILL
				//VEOF
				//VTIME
				//VMIN
				//VSWTC
				//VSTART
				//VSTOP
				//VSUSP
				//VEOL
				//VREPRINT
				//VDISCARD
				//VWERASE
				//VLNEXT
				//VEOL2 
			}
			if(m_TermIOs.c_lflag & linux::ECHO)
			{
				if(!bSigSent)
				{
					if(m_TermIOs.c_lflag & linux::ECHOE)
						WriteFile(m_hConsoleWrite, "\b \b", 3, &dwWritten, NULL); //backspace-space-backspace
					else
						WriteFile(m_hConsoleWrite, p, 1, &dwWritten, NULL); //echo char
				}
			}

			//Output settings: c_oflag;
			//not for input :-)

		}

		//make data available to processes
		WriteFile(m_hConsoleReadBufferWrite, buf, dwRead, &dwWritten, NULL);
	}
	return 0;
}


DWORD DevConsole::ioctl(DWORD request, DWORD data)
{
	DWORD dwRet = 0;
	DWORD dwDone;
	EnterCriticalSection(&m_csIoctl); //only one ioctl at a time (because we need read/write to match)

	//the actual work needs to happen in the console handler process
	//sent request there and read rely if needed

	switch(request)
	{
	case linux::TCGETS: //get stty stuff
		//send to process
		P->WriteMemory((ADDR)data, sizeof(m_TermIOs), &m_TermIOs);
		break;

	case linux::TCSETSW:
		//flush output
		//console io never buffered, so not required
		//...fall through...
	case linux::TCSETSF:
		//flush input & output
		//(done in console process)
		//...fall through...
	case linux::TCSETS: //set stty stuff
		{
			//do this in kernel
			P->ReadMemory(&m_TermIOs, (ADDR)data, sizeof(m_TermIOs));

			//console does some of it
			WriteFile(m_hIoctlWrite, &request, sizeof(DWORD), &dwDone,0);
		}
		break;

	case linux::TIOCGPGRP: //get process group
		{
			if(!data)
			{
				dwRet = -linux::EFAULT;
				break;
			}
			P->WriteMemory((ADDR)data, sizeof(linux::pid_t), &m_ProcessGroup);
		}
		break;
	case linux::TIOCSPGRP: //set process group
		{
			if(!data)
			{
				dwRet = -linux::EFAULT;
				break;
			}
			P->ReadMemory(&m_ProcessGroup, (ADDR)data, sizeof(linux::pid_t));
		}
		break;

	case linux::TIOCGWINSZ: //get window size
		{
			linux::winsize rec;
			WriteFile(m_hIoctlWrite, &request, sizeof(DWORD), &dwDone,0);
			ReadFile(m_hIoctlRead, &rec, sizeof(rec), &dwDone,0);

			//send to process
			P->WriteMemory((ADDR)data, sizeof(rec), &rec);
		}
		break;
	case linux::TIOCSWINSZ: //set window size
		{
			linux::winsize rec;

			P->ReadMemory(&rec, (ADDR)data, sizeof(rec));

			WriteFile(m_hIoctlWrite, &request, sizeof(DWORD), &dwDone,0);
			WriteFile(m_hIoctlWrite, &rec, sizeof(rec), &dwDone,0);
		}
		break;

	case linux::TCXONC: //TODO:
		dwRet = 0;
		break;

	default:
		ktrace("IMPLEMENT sys_ioctl 0x%lx for DevConsole\n", request);
		dwRet = -linux::ENOSYS;
		break;
	}

	LeaveCriticalSection(&m_csIoctl);
	return dwRet;
}

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

#include "IOHPipe.h"
#include "FilesystemKeow.h"


// eax is the syscall number
// ebx,ecx,edx,esi,edi,ebp are up to 6(max) parameters
// any more parameters and the caller just puts a struct pointer in one of these
// eax is the return value


/*
 * write bytes to a handle
 * sys_write(handle,text,len)
 */
void SysCalls::sys_write(CONTEXT& ctx)
{
	int fd;
	IOHandler * ioh;
	
	fd = ctx.Ebx;
	if(fd<0 || fd>MAX_OPEN_FILES)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ioh = P->m_OpenFiles[fd];
	if(ioh == NULL)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	if(!ioh->Write((void*)ctx.Ecx, ctx.Edx, &ctx.Eax))
	{
		DWORD err = SysCallDll::GetLastError();
		ctx.Eax = -Win32ErrToUnixError(SysCallDll::GetLastError());
		switch(err)
		{
		case ERROR_BROKEN_PIPE:
		case ERROR_NO_DATA: //pipe is being closed
		case ERROR_PIPE_NOT_CONNECTED:
			P->SendSignal(linux::SIGPIPE);
			break;
		}
	}
}

/*
 * int writev(int filedes, struct iovec* v, int count)
 * this is a write from several buffers
 */
void SysCalls::sys_writev(CONTEXT& ctx)
{
	linux::iovec *pV = (linux::iovec *)ctx.Ecx;
	linux::iovec iov;
	int count = ctx.Edx;
	int fd;
	IOHandler * ioh;
	
	fd = ctx.Ebx;
	if(fd<0 || fd>MAX_OPEN_FILES)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ioh = P->m_OpenFiles[fd];
	if(ioh == NULL)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	DWORD total=0;
	DWORD done;
	for(int i=0; i<count; ++i)
	{
		P->ReadMemory(&iov, (ADDR)pV, sizeof(iov));

		if(!ioh->Write(iov.iov_base, iov.iov_len, &done))
		{
			DWORD err = SysCallDll::GetLastError();
			ctx.Eax = -Win32ErrToUnixError(SysCallDll::GetLastError());
			switch(err)
			{
			case ERROR_BROKEN_PIPE:
			case ERROR_NO_DATA: //pipe is being closed
			case ERROR_PIPE_NOT_CONNECTED:
				P->SendSignal(linux::SIGPIPE);
				break;
			}
			return;
		}

		total += done;
		++pV;
	}

	ctx.Eax = total;
}

/*****************************************************************************/

/*
 * unsigned long sys_read(int fd, char*buf, unsigned long size)
 */
void SysCalls::sys_read(CONTEXT& ctx)
{
	int fd;
	IOHandler * ioh;

	ktrace("read(fd %d, len %d into 0x%08lx)\n", ctx.Ebx, ctx.Edx, ctx.Ecx);

	fd = ctx.Ebx;
	if(fd<0 || fd>MAX_OPEN_FILES)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ioh = P->m_OpenFiles[fd];
	if(ioh == NULL)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	if(!ioh->Read((void*)ctx.Ecx, ctx.Edx, &ctx.Eax))
		ctx.Eax = -Win32ErrToUnixError(SysCallDll::GetLastError());
}

/*****************************************************************************/

/*
 * access(filename, perms)
 */
void SysCalls::sys_access(CONTEXT& ctx)
{
	Path p;
	DWORD attr;
	char ok = 1;
	DWORD check = ctx.Ecx;

	p.SetUnixPath( MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx) );

	attr = GetFileAttributes(p.GetWin32Path());
	if(attr==INVALID_FILE_ATTRIBUTES)
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
		return;
	}

	//read is ok if access already succeeded

	if( (check & linux::W_OK) && (attr&FILE_ATTRIBUTE_READONLY) )
		ok = 0;

	if(ok)
		ctx.Eax = 0;
	else
		ctx.Eax = -linux::EACCES;
}

/*****************************************************************************/


/*
 * int open(path, access, perms)
 * open or create a file
 */
void SysCalls::sys_open(CONTEXT& ctx)
{
	Path p;
	DWORD access = ctx.Ecx;
	DWORD perms = ctx.Edx;
	int fd;
	DWORD win32access, win32share, disposition, flags;
	IOHandler * ioh;

	string s = MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx);
	ktrace("open([0x%08lx]%s, 0x%lx, 0%lo)\n", ctx.Ebx,s.c_str(), access, perms);
	p.SetUnixPath(s);

	//find free handle entry
	fd = P->FindFreeFD();
	if(fd==-1)
	{
		ctx.Eax = -linux::EMFILE; //too many open files
		return;
	}

	//calc flags
	if((access&linux::O_ACCMODE) == linux::O_WRONLY)
		win32access = GENERIC_WRITE;
	else
		if((access&linux::O_ACCMODE) == linux::O_RDONLY)
			win32access = GENERIC_READ;
		else
			win32access = GENERIC_READ|GENERIC_WRITE;
	if(access & linux::O_EXCL)
		win32share = 0;
	else
		win32share = FILE_SHARE_READ|FILE_SHARE_WRITE;
	disposition = OPEN_EXISTING;
	if(access & linux::O_CREAT)
		disposition = OPEN_ALWAYS;
	flags = 0;


	//open
	ioh = IOHandler::CreateForPath(p);
	if(ioh==NULL)
	{
		ctx.Eax = -linux::ENXIO; //no device available?
		return;
	}

	bool ok = ioh->Open(win32access, win32share, disposition, flags);
	if(!ok)
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
		delete ioh;
		return;
	}


	P->m_OpenFiles[fd] = ioh;
	ctx.Eax = fd;

	//more flags
	if(access & linux::O_TRUNC) 
	{
		ioh->Seek(0, FILE_BEGIN);
		ioh->Truncate();
	}
	if(access & linux::O_APPEND)
	{
		ioh->Seek(0, FILE_END);
	}

}


/*****************************************************************************/


/*
 * int close(handle)
 */
void SysCalls::sys_close(CONTEXT& ctx)
{
	int fd;
	IOHandler * ioh;
	
	fd = ctx.Ebx;
	if(fd<0 || fd>MAX_OPEN_FILES)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ioh = P->m_OpenFiles[fd];
	if(ioh == NULL)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	//free (does the close too)
	delete ioh;
	P->m_OpenFiles[fd] = NULL;

	ctx.Eax = 0;
}


/*****************************************************************************/


/*
 * int select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
 */
void SysCalls::sys_select(CONTEXT& ctx)
{
	sys__newselect(ctx);
}


/*****************************************************************************/


/*
 * int ioctl(int fd, int request, ...)
 */
void SysCalls::sys_ioctl(CONTEXT& ctx)
{
	int fd;
	IOHandler * ioh;
	
	fd = ctx.Ebx;
	if(fd<0 || fd>MAX_OPEN_FILES)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ioh = P->m_OpenFiles[fd];
	if(ioh == NULL)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ctx.Eax = ioh->ioctl( ctx.Ecx, ctx.Edx );
}


/*****************************************************************************/


/*
 * int getcwd(char* buf, int size)
 */
void SysCalls::sys_getcwd(CONTEXT& ctx)
{
	int cnt = (int)ctx.Ecx;
	string cwd = P->m_UnixPwd.GetUnixPath();
	if(cnt < 1+cwd.length())
	{
		cnt = cwd.length();
		cwd[cnt-1] = 0;
	}
	P->WriteMemory((ADDR)ctx.Ebx, cnt, (char*)cwd.c_str());
	ctx.Eax = 0;
}



/*****************************************************************************/


/*
 * int chdir(const char* path)
 */
void SysCalls::sys_chdir(CONTEXT& ctx)
{
	Path p;
	p.SetUnixPath( MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx) );

	//it is a directory, right?
	DWORD attr;
	attr = GetFileAttributes(p.GetWin32Path());
	if(attr==INVALID_FILE_ATTRIBUTES)
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
		return;
	}
	if((attr&FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		ctx.Eax = -linux::ENOTDIR;
		return;
	}


	//seems to be a directory - use it
	ktrace("chdir %s\n", p.GetUnixPath());
	P->m_UnixPwd = p;

	ctx.Eax = 0;
}

/*
 * int fchdir(int fd)
 */
void SysCalls::sys_fchdir(CONTEXT& ctx)
{
	int fd;
	IOHandler * ioh;

	fd = ctx.Ebx;
	if(fd<0 || fd>MAX_OPEN_FILES)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ioh = P->m_OpenFiles[fd];
	if(ioh == NULL
	|| !instanceof(ioh,IOHFile))
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	//it is a directory, right?
	Path& path = ((IOHFile*)ioh)->GetPath();
	DWORD attr;
	attr = GetFileAttributes(path.GetWin32Path());
	if(attr==INVALID_FILE_ATTRIBUTES)
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
		return;
	}
	if((attr&FILE_ATTRIBUTE_DIRECTORY) == 0)
	{
		ctx.Eax = -linux::ENOTDIR;
		return;
	}


	//seems to be a directory - use it
	ktrace("chdir %s\n", path.GetUnixPath());
	P->m_UnixPwd = path;

	ctx.Eax = 0;
}


/*****************************************************************************/


/*
 * int fcntl(int fd, int cmd, ...)
 */
void SysCalls::sys_fcntl(CONTEXT& ctx)
{
	int fd;
	IOHandler * ioh;
	
	fd = ctx.Ebx;
	if(fd<0 || fd>MAX_OPEN_FILES)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ioh = P->m_OpenFiles[fd];
	if(ioh == NULL)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ctx.Eax = -linux::EINVAL;

	switch(ctx.Ecx)
	{
	case linux::F_GETFD:	/* get close_on_exec */
		ctx.Eax = ioh->GetInheritable() ? 0 : 1;
		break;

	case linux::F_SETFD:   /* set/clear close_on_exec */
		ioh->SetInheritable(ctx.Edx==0);
		ctx.Eax = 0;
		break;

	case linux::F_GETFL:	/* get file->f_flags */
		ctx.Eax = ioh->GetFlags();
		break;

	case linux::F_SETFL:	/* set file->f_flags */
		ioh->SetFlags(ctx.Edx);
		ctx.Eax = 0;
		break;

	case linux::F_DUPFD:	/* like dup2() but use any fd<=arg and close-on-exec flag of copy is off */
		{
			int maxfd = ctx.Edx;
			int fdnew;
			DWORD OldEcx;

			//find free handle entry
			fdnew = P->FindFreeFD();
			if(fdnew==-1)
			{
				ctx.Eax = -linux::EMFILE; //too many open files
				return;
			}
			if(fdnew > maxfd)
			{
				ctx.Eax = linux::EINVAL;
				return;
			}

			//use dup2 for actual work
			OldEcx = ctx.Ecx;
			ctx.Ecx = fdnew;
			sys_dup2(ctx);
			ctx.Ecx = OldEcx;
			P->m_OpenFiles[fdnew]->SetInheritable(false);
			//eax is already set with the result
		}
		break;

	default:
		ktrace("IMPLEMENT sys_fcntl 0x%lx\n", ctx.Ecx);
		ctx.Eax = -linux::ENOSYS;
		break;
	}
}


/*****************************************************************************/


/*
 * int dup(int fd)
 */
void SysCalls::sys_dup(CONTEXT& ctx)
{
	int fdnew;
	DWORD OldEcx;

	//find free handle entry
	fdnew = P->FindFreeFD();
	if(fdnew==-1)
	{
		ctx.Eax = -linux::EMFILE; //too many open files
		return;
	}

	//use dup2 for actual work
	OldEcx = ctx.Ecx; //save
	ctx.Ecx = fdnew;
	sys_dup2(ctx);
	ctx.Ecx = OldEcx; //restore
}


/*****************************************************************************/


/*
 * int dup2(int fd, int newfd)
 */
void SysCalls::sys_dup2(CONTEXT& ctx)
{
	int fdold, fdnew;
	IOHandler *ioh_old;

	//validate old handle
	fdold = ctx.Ebx;
	if(fdold<0 || fdold>MAX_OPEN_FILES)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ioh_old = P->m_OpenFiles[fdold];
	if(ioh_old == NULL)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	//validate new handle
	fdnew = ctx.Ecx;
	if(fdnew<0 || fdnew>MAX_OPEN_FILES || fdnew==fdold)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	//close any existing
	if(P->m_OpenFiles[fdnew] != NULL)
	{
		P->m_OpenFiles[fdnew]->Close();
		delete P->m_OpenFiles[fdnew];
		P->m_OpenFiles[fdnew] = NULL;
	}
	//make duplicate
	IOHandler *ioh_new = ioh_old->Duplicate();
	if(ioh_new==NULL)
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
		return;
	}
	P->m_OpenFiles[fdnew] = ioh_new;
	ctx.Eax = fdnew;
}


/*****************************************************************************/


/*
 * int getdents64(int fd, dirent64* d, int bytes)
 */
void SysCalls::sys_getdents64(CONTEXT& ctx)
{
	linux::dirent64 * s = (linux::dirent64 *)ctx.Ecx;
	int maxbytes = ctx.Edx;
	int filled = 0;
	int fd;
	IOHandler * ioh;

	fd = ctx.Ebx;
	if(fd<0 || fd>MAX_OPEN_FILES)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ktrace("getdents64(fd %d, 0x%08lx, max %d)\n", fd, s, maxbytes);

	ioh = P->m_OpenFiles[fd];
	if(ioh == NULL)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	//read locally, copy to process
	LPBYTE buf = new BYTE[maxbytes];

	filled = ioh->GetDirEnts64((linux::dirent64 *)buf, maxbytes);
	if(filled >= 0)
	{
		if(filled>0)
			P->WriteMemory((ADDR)s, filled, buf);
		ctx.Eax = filled;
	}
	else
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
	}
	
	delete buf;
}


/*****************************************************************************/


/*
 * int stat(const char * name,  struct stat* buf)
 */
void SysCalls::sys_stat(CONTEXT& ctx)
{
	string fname = MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx);
	linux::stat * pBuf = (linux::stat*)ctx.Ecx;
	linux::stat buf;

	//DO resolve symbolic links for stat
	Path p(true);
	p.SetUnixPath(fname);

	IOHandler * ioh = IOHandler::CreateForPath(p);
	if(ioh==NULL)
	{
		ctx.Eax = -linux::ENOMEM;
		return;
	}

	if(ioh->Stat(&buf))
	{
		P->WriteMemory((ADDR)pBuf, sizeof(buf), &buf);
		ctx.Eax = 0;
	}
	else
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
	}

	delete ioh;
}


/*
 * int lstat(const char * name,  struct stat* buf)
 */
void SysCalls::sys_lstat(CONTEXT& ctx)
{
	string fname = MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx);
	linux::stat * pBuf = (linux::stat*)ctx.Ecx;
	linux::stat buf;

	//don't resolve symbolic links for lstat
	Path p(false);
	p.SetUnixPath(fname);

	IOHandler * ioh = IOHandler::CreateForPath(p);
	if(ioh==NULL)
	{
		ctx.Eax = -linux::ENOMEM;
		return;
	}

	if(ioh->Stat(&buf))
	{
		P->WriteMemory((ADDR)pBuf, sizeof(buf), &buf);
		ctx.Eax = 0;
	}
	else
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
	}

	delete ioh;
}


/*
 * int fstat(int fd,  struct stat* buf)
 */
void SysCalls::sys_fstat(CONTEXT& ctx)
{
	linux::stat * pBuf = (linux::stat*)ctx.Ecx;
	linux::stat buf;
	int fd;
	IOHandler * ioh;

	fd = ctx.Ebx;
	if(fd<0 || fd>MAX_OPEN_FILES)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ioh = P->m_OpenFiles[fd];
	if(ioh == NULL)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	if(ioh->Stat(&buf))
	{
		P->WriteMemory((ADDR)pBuf, sizeof(buf), &buf);
		ctx.Eax = 0;
	}
	else
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
	}
}


/*****************************************************************************/


/*
 * int stat64(const char * name,  struct stat64* buf)
 */
void SysCalls::sys_stat64(CONTEXT& ctx)
{
	string fname = MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx);
	linux::stat64 * pBuf = (linux::stat64*)ctx.Ecx;
	linux::stat64 buf;

	//DO resolve symbolic links for stat
	Path p(true);
	p.SetUnixPath(fname);

	IOHandler * ioh = IOHandler::CreateForPath(p);
	if(ioh==NULL)
	{
		ctx.Eax = -linux::ENOMEM;
		return;
	}

	if(ioh->Stat64(&buf))
	{
		P->WriteMemory((ADDR)pBuf, sizeof(buf), &buf);
		ctx.Eax = 0;
	}
	else
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
	}

	delete ioh;
}


/*
 * int lstat64(const char * name,  struct stat64* buf)
 */
void SysCalls::sys_lstat64(CONTEXT& ctx)
{
	string fname = MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx);
	linux::stat64 * pBuf = (linux::stat64*)ctx.Ecx;
	linux::stat64 buf;

	//don't resolve symbolic links for lstat
	Path p(false);
	p.SetUnixPath(fname);

	IOHandler * ioh = IOHandler::CreateForPath(p);
	if(ioh==NULL)
	{
		ctx.Eax = -linux::ENOMEM;
		return;
	}

	if(ioh->Stat64(&buf))
	{
		P->WriteMemory((ADDR)pBuf, sizeof(buf), &buf);
		ctx.Eax = 0;
	}
	else
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
	}

	delete ioh;
}


/*
 * int fstat64(int fd,  struct stat64* buf)
 */
void SysCalls::sys_fstat64(CONTEXT& ctx)
{
	linux::stat64 * pBuf = (linux::stat64*)ctx.Ecx;
	linux::stat64 buf;
	int fd;
	IOHandler * ioh;

	fd = ctx.Ebx;
	if(fd<0 || fd>MAX_OPEN_FILES)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ioh = P->m_OpenFiles[fd];
	if(ioh == NULL)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	if(ioh->Stat64(&buf))
	{
		P->WriteMemory((ADDR)pBuf, sizeof(buf), &buf);
		ctx.Eax = 0;
	}
	else
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
	}
}


/*****************************************************************************/

/*
 * int readlink(path, buf, size)
 */
void SysCalls::sys_readlink(CONTEXT& ctx)
{
	//don't follow symlinks
	Path p;
	p.FollowSymLinks(false);
	p.SetUnixPath( MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx) );

	if(GetFileAttributes(p.GetWin32Path())==INVALID_FILE_ATTRIBUTES)
	{
		ctx.Eax = -linux::ENOENT; //no file
	}
	else
	if(!p.IsSymbolicLink())
	{
		ctx.Eax = -linux::EINVAL; //not a link
	}
	else
	{
		//want to follow it now
		p.FollowSymLinks(true);

		//readlink does not copy the null!
		char *pDest = (char*)ctx.Ecx;
		string up = p.GetUnixPath();
		int max = (int)ctx.Edx;
		if(max>up.length())
			max = up.length();

		P->WriteMemory((ADDR)pDest, max, up.c_str());
		ctx.Eax = max; //chars copied
	}
}


/*****************************************************************************/

/*
 * int pipe(int filesdes[2])
 */
void SysCalls::sys_pipe(CONTEXT& ctx)
{
	//handles to open
	HANDLE hRead, hWrite;

	if(!CreatePipe(&hRead, &hWrite, NULL, 0))
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
		return;
	}

	//make fd entries
	int fds[2];
	fds[0] = P->FindFreeFD();
	P->m_OpenFiles[fds[0]] = new IOHPipe(hRead);
	fds[1] = P->FindFreeFD();
	P->m_OpenFiles[fds[1]] = new IOHPipe(hWrite);

	//return fd entries
	P->WriteMemory((ADDR)ctx.Ebx, sizeof(int)*2, fds);

	ctx.Eax = 0;
}


/*****************************************************************************/

/*
 * int link(olfpath, newpath)
 */
void SysCalls::sys_link(CONTEXT& ctx)
{
	Path OldP(false), NewP(false);

	OldP.SetUnixPath( MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx) );
	NewP.SetUnixPath( MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ecx) );

	if(LegacyWindows::CreateHardLink(NewP.GetWin32Path(), OldP.GetWin32Path()))
		ctx.Eax = 0;
	else
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
}


/*
 * int unlink(path)
 */
void SysCalls::sys_unlink(CONTEXT& ctx)
{
	Path p(false);
	p.SetUnixPath( MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx) );

	ktrace("unlink(%s)\n", p.GetWin32Path().c_str() );

	DWORD attr = GetFileAttributes(p.GetWin32Path());
	if(attr==INVALID_FILE_ATTRIBUTES)
	{
		ctx.Eax = -linux::ENOENT;
		return;
	}
	if(attr & FILE_ATTRIBUTE_DIRECTORY)
	{
		ctx.Eax = -linux::EISDIR;
		return;
	}


	if(DeleteFile(p.GetWin32Path()))
	{
		ctx.Eax = 0;
		return;
	}

	DWORD err = GetLastError();
	ctx.Eax = -Win32ErrToUnixError(err);

	if(err!=ERROR_SHARING_VIOLATION)
		return; //return this error


	//file is still being used. In unix you can delete such a file
	//but not in windows. However in windows it can often be successfully
	//renamed within the same directory, so try to do this with a temporary
	//name and retry later
	/*
	for(int i=0; i<10000; ++i) //if 10000 files have been deleted we will fail - on well :-)
	{
		char p2[MAX_PATH];

	}
	*/
	return; //return the original error
}


/*
 * int symlink(const char *oldpath, const char *newpath)
 */
void SysCalls::sys_symlink(CONTEXT& ctx)
{
	string OldP = MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx);
	string NewP = MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ecx);

	if(FilesystemKeow::CreateSymbolicLink(OldP, NewP))
		ctx.Eax = 0;
	else
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
}

/*****************************************************************************/

/*
 * int _llseek(int fd, ulong offset_high, ulong offset_low, loff_t* result, uint whence)
 */
void SysCalls::sys__llseek(CONTEXT& ctx)	
{
	int fd = ctx.Ebx;
	LONG offset_high = ctx.Ecx;
	LONG offset_low = ctx.Edx;
	linux::loff_t * pResult = (linux::loff_t*)ctx.Esi;
	DWORD whence = ctx.Edi;

	IOHandler * ioh;

	fd = ctx.Ebx;
	if(fd<0 || fd>MAX_OPEN_FILES)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ioh = P->m_OpenFiles[fd];
	if(ioh == NULL)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	DWORD method;
	if(whence==linux::_SEEK_SET)
		method=FILE_BEGIN;
	else
	if(whence==linux::_SEEK_END)
		method=FILE_END;
	else
	if(whence==linux::_SEEK_CUR)
		method=FILE_CURRENT;
	else
	{
		ctx.Eax = -linux::EINVAL;
		return;
	}


	LARGE_INTEGER li;
	li.LowPart = offset_low;
	li.HighPart = offset_high;

	linux::loff_t result = ioh->Seek(li.QuadPart, method);

	P->WriteMemory((ADDR)pResult, sizeof(result), &result);
	ctx.Eax = -Win32ErrToUnixError(SysCallDll::GetLastError()); //may = 0 = success
}


/*
 * off_t lseek(int fildes, off_t offset, int whence)
 */
void SysCalls::sys_lseek(CONTEXT& ctx)
{
	int fd;
	IOHandler * ioh;
	DWORD offset = ctx.Ecx;
	DWORD whence = ctx.Edx;

	fd = ctx.Ebx;
	if(fd<0 || fd>MAX_OPEN_FILES)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	ioh = P->m_OpenFiles[fd];
	if(ioh == NULL)
	{
		ctx.Eax = -linux::EBADF;
		return;
	}

	DWORD method;
	if(whence==SEEK_SET)
		method=FILE_BEGIN;
	else
	if(whence==SEEK_END)
		method=FILE_END;
	else
	if(whence==SEEK_CUR)
		method=FILE_CURRENT;
	else
	{
		ctx.Eax = -linux::EINVAL;
		return;
	}


	DWORD result = (DWORD)ioh->Seek(offset, method);
	ctx.Eax = -Win32ErrToUnixError(SysCallDll::GetLastError());
	if(ctx.Eax==0)
		ctx.Eax = result;
}


/*****************************************************************************/

/*
 * int select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout)
 */
void SysCalls::sys__newselect(CONTEXT& ctx)
{
	int numFds = ctx.Ebx;
	linux::fd_set *pReadFds = (linux::fd_set*)ctx.Ecx;
	linux::fd_set *pWriteFds = (linux::fd_set*)ctx.Edx;
	linux::fd_set *pExceptFds = (linux::fd_set*)ctx.Esi;
	linux::timeval *pTimeout = (linux::timeval*)ctx.Edi;

	linux::fd_set ReadRequest, WriteRequest, ExceptRequest;
	KEOW_FD_ZERO(&ReadRequest);
	KEOW_FD_ZERO(&WriteRequest);
	KEOW_FD_ZERO(&ExceptRequest);
	if(pReadFds)
		P->ReadMemory(&ReadRequest, (ADDR)pReadFds, sizeof(ReadRequest));
	if(pWriteFds)
		P->ReadMemory(&WriteRequest, (ADDR)pWriteFds, sizeof(WriteRequest));
	if(pExceptFds)
		P->ReadMemory(&ExceptRequest, (ADDR)pExceptFds, sizeof(ExceptRequest));

	linux::fd_set ReadResults, WriteResults, ExceptResults;
	KEOW_FD_ZERO(&ReadResults);
	KEOW_FD_ZERO(&WriteResults);
	KEOW_FD_ZERO(&ExceptResults);


	DWORD dwWait;
	if(pTimeout==0) {
		dwWait = INFINITE;
		ktrace("new_select wait forever\n");
	}
	else {
		linux::timeval to;
		P->ReadMemory(&to, (ADDR)pTimeout, sizeof(to));

		dwWait = to.tv_sec*1000L + to.tv_usec/1000; //correct?
		ktrace("new_select wait (%ld,%ld) = %ldms\n", to.tv_sec, to.tv_usec, dwWait);
	}

	DWORD dwEnd = GetTickCount() + dwWait; //wrap around will cause a quick return - oops :-)
	bool foundData = false;
	do
	{
		for(int fd=0; fd<numFds; ++fd)
		{
			if( KEOW_FD_ISSET(fd, &ReadRequest)
			&&  P->m_OpenFiles[fd]!=NULL
			&&  P->m_OpenFiles[fd]->CanRead() ) {
				KEOW_FD_SET(fd, &ReadResults);
				foundData = true;
			}

			if( KEOW_FD_ISSET(fd, &WriteRequest)
			&&  P->m_OpenFiles[fd]!=NULL
			&&  P->m_OpenFiles[fd]->CanWrite() ) {
				KEOW_FD_SET(fd, &WriteResults);
				foundData = true;
			}

			if( KEOW_FD_ISSET(fd, &ExceptRequest)
			&&  P->m_OpenFiles[fd]!=NULL
			&&  P->m_OpenFiles[fd]->HasException() ) {
				KEOW_FD_SET(fd, &ExceptResults);
				foundData = true;
			}
		}
		if(foundData)
			break;

		//loop
		if(dwWait==0)
			break;

		//todo: use proper waits, for now polling is simpler
		//Sleep( (pTimeout->tv_sec*1000L + pTimeout->tv_usec) / 10 );
		Sleep(50);

	} while(dwEnd > GetTickCount());

	ktrace("new_select found data: %d\n", foundData);

	//store results
	if(pReadFds)
		P->WriteMemory((ADDR)pReadFds, sizeof(ReadResults), &ReadResults);
	if(pWriteFds)
		P->WriteMemory((ADDR)pWriteFds, sizeof(WriteResults), &WriteResults);
	if(pExceptFds)
		P->WriteMemory((ADDR)pExceptFds, sizeof(ExceptResults), &ExceptResults);

	if(foundData)
		ctx.Eax = numFds;
	else
		ctx.Eax = 0; //timeout
}

/*****************************************************************************/


/*
 * int mkdir(path)
 * create a new directory
 */
void SysCalls::sys_mkdir(CONTEXT& ctx)
{
	DWORD mode = ctx.Ecx;

	Path p;
	p.SetUnixPath( MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx) );

	if(CreateDirectory(p.GetWin32Path(), NULL))
	{
		ctx.Eax = 0;
		return;
	}

	DWORD err = GetLastError();
	ctx.Eax = -Win32ErrToUnixError(err);
}

/*
 * int rmdir(path)
 * remove a directory - it must be empty
 */
void SysCalls::sys_rmdir(CONTEXT& ctx)
{
	Path p(false);
	p.SetUnixPath( MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx) );

	if(RemoveDirectory(p.GetWin32Path()))
	{
		ctx.Eax = 0;
		return;
	}

	DWORD err = GetLastError();
	ctx.Eax = -Win32ErrToUnixError(err);

	if(err!=ERROR_SHARING_VIOLATION)
		return; //return this error


	//file is still being used. In unix you can delete such a file
	//but not in windows. However in windows it can often be successfully
	//renamed within the same directory, so try to do this with a temporary
	//name and retry later
	/*
	for(int i=0; i<10000; ++i) //if 10000 files have been deleted we will fail - on well :-)
	{
		char p2[MAX_PATH];

	}
	*/
	return; //return the original error
}


/*****************************************************************************/


/*
 * int rename(oldpath, newpath)
 */
void SysCalls::sys_rename(CONTEXT& ctx)
{
	Path OldP, NewP;

	OldP.SetUnixPath( MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx) );
	NewP.SetUnixPath( MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ecx) );

	char * p2 = NULL;
	if(OldP.IsSymbolicLink())
	{
		int len = strlen(p2);
		len+=4;
		p2 = new char[len];
		StringCbPrintf(p2, len, "%s.lnk", NewP.GetWin32Path());
	}

	if(MoveFileEx(OldP.GetWin32Path(), p2?p2:NewP.GetWin32Path(), MOVEFILE_REPLACE_EXISTING))
		ctx.Eax = 0;
	else
		ctx.Eax = -Win32ErrToUnixError(GetLastError());

	if(p2)
		delete p2;
}


/*
 * int utime(const char *filename, struct utimbuf *buf)
 */
void SysCalls::sys_utime(CONTEXT& ctx)
{
	Path path;
	path.SetUnixPath( MemoryHelper::ReadString(P->m_hProcess, (ADDR)ctx.Ebx) );

	linux::utimbuf times;
	P->ReadMemory(&times, (ADDR)ctx.Ecx, sizeof(times));

	DWORD attr = GetFileAttributes(path.GetWin32Path());
	HANDLE hTmp = CreateFile(path.GetWin32Path(), FILE_WRITE_ATTRIBUTES, 0, 0, OPEN_EXISTING, (attr&FILE_ATTRIBUTE_DIRECTORY)?FILE_FLAG_BACKUP_SEMANTICS:0, 0);
	if(hTmp==INVALID_HANDLE_VALUE)
	{
		ctx.Eax = -Win32ErrToUnixError(GetLastError());
		return;
	}


	FILETIME access, modify;
	TIME_T_TO_FILETIME(times.actime, access);
	TIME_T_TO_FILETIME(times.modtime, modify);

	if(SetFileTime(hTmp, NULL, &access, &modify))
		ctx.Eax = 0;
	else
		ctx.Eax = -Win32ErrToUnixError(GetLastError());

	CloseHandle(hTmp);
}


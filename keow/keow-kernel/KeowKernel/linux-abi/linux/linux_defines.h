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
 * This is an include file based on linux kernel sources.
 * It helps describe the user-kernel interface.
 * We have this rather than the actual kernel headers because
 * there are too many #defines that would conflict the windows
 * versions (which are often different values. eg AF_INET6 in sockets).
 * We use consts here so that we get no conflicts but similar names.
 * This is a pair with linux/types.h
 */

namespace linux {


const int __NFDBITS = (8 * sizeof(unsigned long));
const int __FD_SETSIZE = 1024;
const int __FDSET_LONGS  = (__FD_SETSIZE/__NFDBITS);


const int  TCGETS          = 0x5401;
const int  TCSETS          = 0x5402;
const int  TCSETSW         = 0x5403;
const int  TCSETSF         = 0x5404;
const int  TCGETA          = 0x5405;
const int  TCSETA          = 0x5406;
const int  TCSETAW         = 0x5407;
const int  TCSETAF         = 0x5408;
const int  TCSBRK          = 0x5409;
const int  TCXONC          = 0x540A;
const int  TCFLSH          = 0x540B;
const int  TIOCEXCL        = 0x540C;
const int  TIOCNXCL        = 0x540D;
const int  TIOCSCTTY       = 0x540E;
const int  TIOCGPGRP       = 0x540F;
const int  TIOCSPGRP       = 0x5410;
const int  TIOCOUTQ        = 0x5411;
const int  TIOCSTI         = 0x5412;
const int  TIOCGWINSZ      = 0x5413;
const int  TIOCSWINSZ      = 0x5414;
const int  TIOCMGET        = 0x5415;
const int  TIOCMBIS        = 0x5416;
const int  TIOCMBIC        = 0x5417;
const int  TIOCMSET        = 0x5418;

/* c_cc characters */
const int  VINTR = 0;
const int  VQUIT = 1;
const int  VERASE = 2;
const int  VKILL = 3;
const int  VEOF = 4;
const int  VTIME = 5;
const int  VMIN = 6;
const int  VSWTC = 7;
const int  VSTART = 8;
const int  VSTOP = 9;
const int  VSUSP = 10;
const int  VEOL = 11;
const int  VREPRINT = 12;
const int  VDISCARD = 13;
const int  VWERASE = 14;
const int  VLNEXT = 15;
const int  VEOL2 = 16;

/* c_iflag bits */
const int  IGNBRK  = 0000001;
const int  BRKINT  = 0000002;
const int  IGNPAR  = 0000004;
const int  PARMRK  = 0000010;
const int  INPCK   = 0000020;
const int  ISTRIP  = 0000040;
const int  INLCR   = 0000100;
const int  IGNCR   = 0000200;
const int  ICRNL   = 0000400;
const int  IUCLC   = 0001000;
const int  IXON    = 0002000;
const int  IXANY   = 0004000;
const int  IXOFF   = 0010000;
const int  IMAXBEL = 0020000;

/* c_oflag bits */
const int  OPOST   = 0000001;
const int  OLCUC   = 0000002;
const int  ONLCR   = 0000004;
const int  OCRNL   = 0000010;
const int  ONOCR   = 0000020;
const int  ONLRET  = 0000040;
const int  OFILL   = 0000100;
const int  OFDEL   = 0000200;
const int  NLDLY   = 0000400;
const int    NL0   = 0000000;
const int    NL1   = 0000400;
const int  CRDLY   = 0003000;
const int    CR0   = 0000000;
const int    CR1   = 0001000;
const int    CR2   = 0002000;
const int    CR3   = 0003000;
const int  TABDLY  = 0014000;
const int    TAB0  = 0000000;
const int    TAB1  = 0004000;
const int    TAB2  = 0010000;
const int    TAB3  = 0014000;
const int    XTABS = 0014000;
const int  BSDLY   = 0020000;
const int    BS0   = 0000000;
const int    BS1   = 0020000;
const int  VTDLY   = 0040000;
const int    VT0   = 0000000;
const int    VT1   = 0040000;
const int  FFDLY   = 0100000;
const int    FF0   = 0000000;
const int    FF1   = 0100000;

/* c_cflag bit meaning */
const int  CBAUD   = 0010017;
const int   B0     = 0000000 ;        /* hang up */
const int   B50    = 0000001;
const int   B75    = 0000002;
const int   B110   = 0000003;
const int   B134   = 0000004;
const int   B150   = 0000005;
const int   B200   = 0000006;
const int   B300   = 0000007;
const int   B600   = 0000010;
const int   B1200  = 0000011;
const int   B1800  = 0000012;
const int   B2400  = 0000013;
const int   B4800  = 0000014;
const int   B9600  = 0000015;
const int   B19200 = 0000016;
const int   B38400 = 0000017;
const int  EXTA		= B19200;
const int  EXTB		= B38400;
const int  CSIZE   = 0000060;
const int    CS5   = 0000000;
const int    CS6   = 0000020;
const int    CS7   = 0000040;
const int    CS8   = 0000060;
const int  CSTOPB  = 0000100;
const int  CREAD   = 0000200;
const int  PARENB  = 0000400;
const int  PARODD  = 0001000;
const int  HUPCL   = 0002000;
const int  CLOCAL  = 0004000;
const int  CBAUDEX = 0010000;
const int     B57600 = 0010001;
const int    B115200 = 0010002;
const int    B230400 = 0010003;
const int    B460800 = 0010004;
const int    B500000 = 0010005;
const int    B576000 = 0010006;
const int    B921600 = 0010007;
const int   B1000000 = 0010010;
const int   B1152000 = 0010011;
const int   B1500000 = 0010012;
const int   B2000000 = 0010013;
const int   B2500000 = 0010014;
const int   B3000000 = 0010015;
const int   B3500000 = 0010016;
const int   B4000000 = 0010017;
const int  CIBAUD    = 002003600000;  /* input baud rate (not used) */
const int  CMSPAR    = 010000000000;          /* mark or space (stick) parity */
const int  CRTSCTS   = 020000000000;          /* flow control */

/* c_lflag bits */
const int  ISIG    = 0000001;
const int  ICANON  = 0000002;
const int  XCASE   = 0000004;
const int  ECHO    = 0000010;
const int  ECHOE   = 0000020;
const int  ECHOK   = 0000040;
const int  ECHONL  = 0000100;
const int  NOFLSH  = 0000200;
const int  TOSTOP  = 0000400;
const int  ECHOCTL = 0001000;
const int  ECHOPRT = 0002000;
const int  ECHOKE  = 0004000;
const int  FLUSHO  = 0010000;
const int  PENDIN  = 0040000;
const int  IEXTEN  = 0100000;

/* tcflow() and TCXONC use these */
const int  TCOOFF         = 0;
const int  TCOON          = 1;
const int  TCIOFF         = 2;
const int  TCION          = 3;

/* tcflush() and TCFLSH use these */
const int  TCIFLUSH       = 0;
const int  TCOFLUSH       = 1;
const int  TCIOFLUSH      = 2;

/* tcsetattr uses these */
const int  TCSANOW        = 0;
const int  TCSADRAIN      = 1;
const int  TCSAFLUSH      = 2;

const int N_TTY          = 0;


/* These constants are for the segment types stored in the image headers */
const int  PT_NULL    = 0;
const int  PT_LOAD    = 1;
const int  PT_DYNAMIC = 2;
const int  PT_INTERP  = 3;
const int  PT_NOTE    = 4;
const int  PT_SHLIB   = 5;
const int  PT_PHDR    = 6;
const int  PT_LOPROC  = 0x70000000;
const int  PT_HIPROC  = 0x7fffffff;
/* These constants define the different elf file types */
const int  ET_NONE   = 0;
const int  ET_REL    = 1;
const int  ET_EXEC   = 2;
const int  ET_DYN    = 3;
const int  ET_CORE   = 4;
const int  ET_LOPROC = 0xff00;
const int  ET_HIPROC = 0xffff;
/* These constants define the various ELF target machines */
const int  EM_NONE  = 0;
const int  EM_386   = 3;

const int  EI_MAG0      =   0;               /* e_ident[] indexes */
const int  EI_MAG1      =   1;
const int  EI_MAG2      =   2;
const int  EI_MAG3      =   3;
const int  EI_CLASS     =   4;
const int  EI_DATA      =   5;
const int  EI_VERSION   =   6;
const int  EI_PAD       =   7;

const int  ELFMAG0      =   0x7f;            /* EI_MAG */
const int  ELFMAG1      =   'E';
const int  ELFMAG2      =   'L';
const int  ELFMAG3      =   'F';
#define ELFMAG     "\177ELF"
const int  SELFMAG      =   4;

const int  ELFCLASSNONE  =  0;               /* EI_CLASS */
const int  ELFCLASS32    =  1;
const int  ELFCLASS64    =  2;
const int  ELFCLASSNUM   =  3;

const int  ELFDATANONE   =  0;               /* e_ident[EI_DATA] */
const int  ELFDATA2LSB   =  1;
const int  ELFDATA2MSB   =  2;

const int  EV_NONE       =  0;               /* e_version, EI_VERSION */
const int  EV_CURRENT    =  1;
const int  EV_NUM        =  2;


/* Symbolic values for the entries in the auxiliary table
   put on the initial stack */
const int  AT_NULL   = 0;     /* end of vector */
const int  AT_IGNORE = 1;     /* entry should be ignored */
const int  AT_EXECFD = 2;     /* file descriptor of program */
const int  AT_PHDR   = 3;     /* program headers for program */
const int  AT_PHENT  = 4;     /* size of program header entry */
const int  AT_PHNUM  = 5;     /* number of program headers */
const int  AT_PAGESZ = 6;     /* system page size */
const int  AT_BASE   = 7;     /* base address of interpreter */
const int  AT_FLAGS  = 8;     /* flags */
const int  AT_ENTRY  = 9;     /* entry point of program */
const int  AT_NOTELF = 10;    /* program is not ELF */
const int  AT_UID    = 11;    /* real uid */
const int  AT_EUID   = 12;    /* effective uid */
const int  AT_GID    = 13;    /* real gid */
const int  AT_EGID   = 14;    /* effective gid */
const int  AT_PLATFORM = 15;  /* string identifying CPU for optimizations */
const int  AT_HWCAP  = 16;    /* arch dependent hints at CPU capabilities */
const int  AT_CLKTCK = 17;    /* frequency at which times() increments */

/* These constants define the permissions on sections in the program
   header, p_flags. */
const int  PF_R            = 0x4;
const int  PF_W            = 0x2;
const int  PF_X            = 0x1;


const int _NSIG           = 64;
const int _NSIG_BPW       = 32;
const int _NSIG_WORDS     = (_NSIG / _NSIG_BPW);

const int  SIGHUP           = 1;
const int  SIGINT           = 2;
const int  SIGQUIT          = 3;
const int  SIGILL           = 4;
const int  SIGTRAP          = 5;
const int  SIGABRT          = 6;
const int  SIGIOT           = 6;
const int  SIGBUS           = 7;
const int  SIGFPE           = 8;
const int  SIGKILL          = 9;
const int  SIGUSR1         = 10;
const int  SIGSEGV         = 11;
const int  SIGUSR2         = 12;
const int  SIGPIPE         = 13;
const int  SIGALRM         = 14;
const int  SIGTERM         = 15;
const int  SIGSTKFLT       = 16;
const int  SIGCHLD         = 17;
const int  SIGCONT         = 18;
const int  SIGSTOP         = 19;
const int  SIGTSTP         = 20;
const int  SIGTTIN         = 21;
const int  SIGTTOU         = 22;
const int  SIGURG          = 23;
const int  SIGXCPU         = 24;
const int  SIGXFSZ         = 25;
const int  SIGVTALRM       = 26;
const int  SIGPROF         = 27;
const int  SIGWINCH        = 28;
const int  SIGIO           = 29;
const int  SIGPOLL         = SIGIO;
/*
const int  SIGLOST         = 29;
*/
const int  SIGPWR          = 30;
const int  SIGSYS          = 31;
const int  SIGUNUSED       = 31;

/* These should not be considered constants from userland.  */
const int  SIGRTMIN        = 32;
const int  SIGRTMAX        = (_NSIG-1);

const int  SIG_BLOCK         = 0;    /* for blocking signals */
const int  SIG_UNBLOCK       = 1;    /* for unblocking signals */
const int  SIG_SETMASK       = 2;    /* for setting the signal mask */


const int  SA_NOCLDSTOP    =0x00000001;
const int  SA_NOCLDWAIT    =0x00000002;
const int  SA_SIGINFO      =0x00000004;
const int  SA_ONSTACK      =0x08000000;
const int  SA_RESTART      =0x10000000;
const int  SA_NODEFER      =0x40000000;
const int  SA_RESETHAND    =0x80000000;

const int  SA_NOMASK       =SA_NODEFER;
const int  SA_ONESHOT      =SA_RESETHAND;
const int  SA_INTERRUPT    =0x20000000; /* dummy -- ignored */

const int  SA_RESTORER     =0x04000000;

const int  SI_USER       =  0;               /* sent by kill, sigsend, raise */
const int  SI_KERNEL     =  0x80;            /* sent by the kernel from somewhere */
const int  SI_QUEUE      =  -1;              /* sent by sigqueue */
//const int  SI_TIMER __SI_CODE(__SI_TIMER,-2); /* sent by timer expiration */
const int  SI_MESGQ      =  -3;              /* sent by real time mesq state change */
const int  SI_ASYNCIO    =  -4;              /* sent by AIO completion */
const int  SI_SIGIO      =  -5;              /* sent by queued SIGIO */
const int  SI_TKILL      =  -6;              /* sent by tkill system call */


const int  EPERM            = 1;      /* Operation not permitted */
const int  ENOENT           = 2;      /* No such file or directory */
const int  ESRCH            = 3;      /* No such process */
const int  EINTR            = 4;      /* Interrupted system call */
const int  EIO              = 5;      /* I/O error */
const int  ENXIO            = 6;      /* No such device or address */
const int  E2BIG            = 7;      /* Argument list too long */
const int  ENOEXEC          = 8;      /* Exec format error */
const int  EBADF            = 9;      /* Bad file number */
const int  ECHILD          = 10;      /* No child processes */
const int  EAGAIN          = 11;      /* Try again */
const int  ENOMEM          = 12;      /* Out of memory */
const int  EACCES          = 13;      /* Permission denied */
const int  EFAULT          = 14;      /* Bad address */
const int  ENOTBLK         = 15;      /* Block device required */
const int  EBUSY           = 16;      /* Device or resource busy */
const int  EEXIST          = 17;      /* File exists or space (stick) parity */
const int  EXDEV           = 18;      /* Cross-device link */ 
const int  ENODEV          = 19;      /* No such device */
const int  ENOTDIR         = 20;      /* Not a directory */
const int  EISDIR          = 21;      /* Is a directory */
const int  EINVAL          = 22;      /* Invalid argument */
const int  ENFILE          = 23;      /* File table overflow */
const int  EMFILE          = 24;      /* Too many open files */
const int  ENOTTY          = 25;      /* Not a typewriter */
const int  ETXTBSY         = 26;      /* Text file busy */
const int  EFBIG           = 27;      /* File too large */
const int  ENOSPC          = 28;      /* No space left on device */
const int  ESPIPE          = 29;      /* Illegal seek */
const int  EROFS           = 30;      /* Read-only file system */
const int  EMLINK          = 31;      /* Too many links */
const int  EPIPE           = 32;      /* Broken pipe */
const int  EDOM            = 33;      /* Math argument out of domain of func */
const int  ERANGE          = 34;      /* Math result not representable */
const int  EDEADLK         = 35;      /* Resource deadlock would occur */
const int  ENAMETOOLONG    = 36;      /* File name too long */
const int  ENOLCK          = 37;      /* No record locks available */
const int  ENOSYS          = 38;      /* Function not implemented */
const int  ENOTEMPTY       = 39;      /* Directory not empty */
const int  ELOOP           = 40;      /* Too many symbolic links encountered */
const int  EWOULDBLOCK     = EAGAIN;  /* Operation would block */
const int  ENOMSG          = 42;      /* No message of desired type */
const int  EIDRM           = 43;      /* Identifier removed */
const int  ECHRNG          = 44;      /* Channel number out of range */
const int  EL2NSYNC        = 45;      /* Level = 2; not synchronized */
const int  EL3HLT          = 46;      /* Level = 3; halted */
const int  EL3RST          = 47;      /* Level = 3; reset */
const int  ELNRNG          = 48;      /* Link number out of range */
const int  EUNATCH         = 49;      /* Protocol driver not attached */
const int  ENOCSI          = 50;      /* No CSI structure available */
const int  EL2HLT          = 51;      /* Level = 2; halted */
const int  EBADE           = 52;      /* Invalid exchange */
const int  EBADR           = 53;      /* Invalid request descriptor */
const int  EXFULL          = 54;      /* Exchange full */
const int  ENOANO          = 55;      /* No anode */
const int  EBADRQC         = 56;      /* Invalid request code */
const int  EBADSLT         = 57;      /* Invalid slot */

const int  EDEADLOCK       = EDEADLK;

const int  EBFONT          = 59;      /* Bad font file format */
const int  ENOSTR          = 60;      /* Device not a stream */
const int  ENODATA         = 61;      /* No data available */
const int  ETIME           = 62;      /* Timer expired */
const int  ENOSR           = 63;      /* Out of streams resources */
const int  ENONET          = 64;      /* Machine is not on the network */
const int  ENOPKG          = 65;      /* Package not installed */
const int  EREMOTE         = 66;      /* Object is remote */
const int  ENOLINK         = 67;      /* Link has been severed */
const int  EADV            = 68;      /* Advertise error */
const int  ESRMNT          = 69;      /* Srmount error */
const int  ECOMM           = 70;      /* Communication error on send */
const int  EPROTO          = 71;      /* Protocol error */
const int  EMULTIHOP       = 72;      /* Multihop attempted */
const int  EDOTDOT         = 73;      /* RFS specific error */
const int  EBADMSG         = 74;      /* Not a data message */
const int  EOVERFLOW       = 75;      /* Value too large for defined data type */
const int  ENOTUNIQ        = 76;      /* Name not unique on network */
const int  EBADFD          = 77;      /* File descriptor in bad state */
const int  EREMCHG         = 78;      /* Remote address changed */
const int  ELIBACC         = 79;      /* Can not access a needed shared library */
const int  ELIBBAD         = 80;      /* Accessing a corrupted shared library */
const int  ELIBSCN         = 81;      /* .lib section in a.out corrupted */
const int  ELIBMAX         = 82;      /* Attempting to link in too many shared libraries */
const int  ELIBEXEC        = 83;      /* Cannot exec a shared library directly */
const int  EILSEQ          = 84;      /* Illegal byte sequence */
const int  ERESTART        = 85;      /* Interrupted system call should be restarted */
const int  ESTRPIPE        = 86;      /* Streams pipe error */
const int  EUSERS          = 87;      /* Too many users */
const int  ENOTSOCK        = 88;      /* Socket operation on non-socket */
const int  EDESTADDRREQ    = 89;      /* Destination address required */
const int  EMSGSIZE        = 90;      /* Message too long */
const int  EPROTOTYPE      = 91;      /* Protocol wrong type for socket */
const int  ENOPROTOOPT     = 92;      /* Protocol not available */
const int  EPROTONOSUPPORT = 93;      /* Protocol not supported */
const int  ESOCKTNOSUPPORT = 94;      /* Socket type not supported */
const int  EOPNOTSUPP      = 95;      /* Operation not supported on transport endpoint */
const int  EPFNOSUPPORT    = 96;      /* Protocol family not supported */
const int  EAFNOSUPPORT    = 97;      /* Address family not supported by protocol */
const int  EADDRINUSE      = 98;      /* Address already in use */
const int  EADDRNOTAVAIL   = 99;      /* Cannot assign requested address */
const int  ENETDOWN        = 100;     /* Network is down */
const int  ENETUNREACH     = 101;     /* Network is unreachable */
const int  ENETRESET       = 102;     /* Network dropped connection because of reset */
const int  ECONNABORTED    = 103;     /* Software caused connection abort */
const int  ECONNRESET      = 104;     /* Connection reset by peer */
const int  ENOBUFS         = 105;     /* No buffer space available */
const int  EISCONN         = 106;     /* Transport endpoint is already connected */
const int  ENOTCONN        = 107;     /* Transport endpoint is not connected */
const int  ESHUTDOWN       = 108;     /* Cannot send after transport endpoint shutdown */
const int  ETOOMANYREFS    = 109;     /* Too many references: cannot splice */
const int  ETIMEDOUT       = 110;     /* Connection timed out */
const int  ECONNREFUSED    = 111;     /* Connection refused */
const int  EHOSTDOWN       = 112;     /* Host is down */
const int  EHOSTUNREACH    = 113;     /* No route to host */
const int  EALREADY        = 114;     /* Operation already in progress */
const int  EINPROGRESS     = 115;     /* Operation now in progress */
const int  ESTALE          = 116;     /* Stale NFS file handle */
const int  EUCLEAN         = 117;     /* Structure needs cleaning */
const int  ENOTNAM         = 118;     /* Not a XENIX named type file */
const int  ENAVAIL         = 119;     /* No XENIX semaphores available */
const int  EISNAM          = 120;     /* Is a named type file */
const int  EREMOTEIO       = 121;     /* Remote I/O error */
const int  EDQUOT          = 122;     /* Quota exceeded */

const int  ENOMEDIUM       = 123;     /* No medium found */
const int  EMEDIUMTYPE     = 124;     /* Wrong medium type */


const int  S_IFMT  =00170000;
const int  S_IFSOCK =0140000;
const int  S_IFLNK  =0120000;
const int  S_IFREG  =0100000;
const int  S_IFBLK  =0060000;
const int  S_IFDIR  =0040000;
const int  S_IFCHR  =0020000;
const int  S_IFIFO  =0010000;
const int  S_ISUID  =0004000;
const int  S_ISGID  =0002000;
const int  S_ISVTX  =0001000;

#define S_ISLNK(m)      (((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)      (((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)      (((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)      (((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)      (((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)     (((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)     (((m) & S_IFMT) == S_IFSOCK)

const int  S_IRWXU = 00700;
const int  S_IRUSR = 00400;
const int  S_IWUSR = 00200;
const int  S_IXUSR = 00100;

const int  S_IRWXG = 00070;
const int  S_IRGRP = 00040;
const int  S_IWGRP = 00020;
const int  S_IXGRP = 00010;

const int  S_IRWXO = 00007;
const int  S_IROTH = 00004;
const int  S_IWOTH = 00002;
const int  S_IXOTH = 00001;


/* open/fcntl - O_SYNC is only implemented on blocks devices and on files
   located on an ext2 file system */
const int  O_ACCMODE     =     0003;
const int  O_RDONLY      =       00;
const int  O_WRONLY      =       01;
const int  O_RDWR        =       02;
const int  O_CREAT       =     0100; /* not fcntl */
const int  O_EXCL        =     0200; /* not fcntl */
const int  O_NOCTTY      =     0400; /* not fcntl */
const int  O_TRUNC       =    01000; /* not fcntl */
const int  O_APPEND      =    02000;
const int  O_NONBLOCK    =    04000;
const int  O_NDELAY      =  O_NONBLOCK;
const int  O_SYNC        =   010000;
const int  FASYNC        =   020000; /* fcntl, for BSD compatibility */
const int  O_DIRECT      =   040000; /* direct disk access hint */
const int  O_LARGEFILE   =  0100000;
const int  O_DIRECTORY   =  0200000; /* must be a directory */
const int  O_NOFOLLOW    =  0400000; /* don't follow links */

const int  F_DUPFD       =  0 ;      /* dup */
const int  F_GETFD       =  1 ;      /* get close_on_exec */
const int  F_SETFD       =  2 ;      /* set/clear close_on_exec */
const int  F_GETFL       =  3 ;      /* get file->f_flags */
const int  F_SETFL       =  4 ;      /* set file->f_flags */
const int  F_GETLK       =  5;
const int  F_SETLK       =  6;
const int  F_SETLKW      =  7;

const int  F_SETOWN      =  8  ;     /*  for sockets. */
const int  F_GETOWN      =  9  ;     /*  for sockets. */
const int  F_SETSIG      =  10 ;     /*  for sockets. */
const int  F_GETSIG      =  11 ;     /*  for sockets. */

const int  F_GETLK64     =  12 ;     /*  using 'struct flock64' */
const int  F_SETLK64     =  13;
const int  F_SETLKW64    =  14;

/* for F_[GET|SET]FL */
const int  FD_CLOEXEC    =  1;       /* actually anything with low bit set goes */

/* for posix fcntl() and lockf() */
const int  F_RDLCK       =  0;
const int  F_WRLCK       =  1;
const int  F_UNLCK       =  2;

/* for old implementation of bsd flock () */
const int  F_EXLCK       =  4 ;      /* or 3 */
const int  F_SHLCK       =  8 ;      /* or 4 */

/* for leases */
const int  F_INPROGRESS  =  16;

/* operations for bsd flock(), also used by the kernel implementation */
const int  LOCK_SH       =  1 ;      /* shared lock */
const int  LOCK_EX       =  2 ;      /* exclusive lock */
const int  LOCK_NB       =  4 ;      /* or'd with one of the above to prevent blocking */
const int  LOCK_UN       =  8 ;      /* remove lock */

const int  LOCK_MAND     =  32 ;     /* This is a mandatory flock */
const int  LOCK_READ     =  64 ;     /* ... Which allows concurrent read operations */
const int  LOCK_WRITE    =  128;     /* ... Which allows concurrent write operations*/
const int  LOCK_RW       =  192;     /* ... Which allows concurrent read & write ops*/

const int  F_LINUX_SPECIFIC_BASE   = 1024;


const int NR_syscalls = 338;

const int  __NR_exit		  = 1;
const int  __NR_fork		  = 2;
const int  __NR_read		  = 3;
const int  __NR_write		  = 4;
const int  __NR_open		  = 5;
const int  __NR_close		  = 6;
const int  __NR_waitpid		  = 7;
const int  __NR_creat		  = 8;
const int  __NR_link		  = 9;
const int  __NR_unlink		 = 10;
const int  __NR_execve		 = 11;
const int  __NR_chdir		 = 12;
const int  __NR_time		 = 13;
const int  __NR_mknod		 = 14;
const int  __NR_chmod		 = 15;
const int  __NR_lchown		 = 16;
const int  __NR_break		 = 17;
const int  __NR_oldstat		 = 18;
const int  __NR_lseek		 = 19;
const int  __NR_getpid		 = 20;
const int  __NR_mount		 = 21;
const int  __NR_umount		 = 22;
const int  __NR_setuid		 = 23;
const int  __NR_getuid		 = 24;
const int  __NR_stime		 = 25;
const int  __NR_ptrace		 = 26;
const int  __NR_alarm		 = 27;
const int  __NR_oldfstat		 = 28;
const int  __NR_pause		 = 29;
const int  __NR_utime		 = 30;
const int  __NR_stty		 = 31;
const int  __NR_gtty		 = 32;
const int  __NR_access		 = 33;
const int  __NR_nice		 = 34;
const int  __NR_ftime		 = 35;
const int  __NR_sync		 = 36;
const int  __NR_kill		 = 37;
const int  __NR_rename		 = 38;
const int  __NR_mkdir		 = 39;
const int  __NR_rmdir		 = 40;
const int  __NR_dup		 = 41;
const int  __NR_pipe		 = 42;
const int  __NR_times		 = 43;
const int  __NR_prof		 = 44;
const int  __NR_brk		 = 45;
const int  __NR_setgid		 = 46;
const int  __NR_getgid		 = 47;
const int  __NR_signal		 = 48;
const int  __NR_geteuid		 = 49;
const int  __NR_getegid		 = 50;
const int  __NR_acct		 = 51;
const int  __NR_umount2		 = 52;
const int  __NR_lock		 = 53;
const int  __NR_ioctl		 = 54;
const int  __NR_fcntl		 = 55;
const int  __NR_mpx		 = 56;
const int  __NR_setpgid		 = 57;
const int  __NR_ulimit		 = 58;
const int  __NR_oldolduname	 = 59;
const int  __NR_umask		 = 60;
const int  __NR_chroot		 = 61;
const int  __NR_ustat		 = 62;
const int  __NR_dup2		 = 63;
const int  __NR_getppid		 = 64;
const int  __NR_getpgrp		 = 65;
const int  __NR_setsid		 = 66;
const int  __NR_sigaction		 = 67;
const int  __NR_sgetmask		 = 68;
const int  __NR_ssetmask		 = 69;
const int  __NR_setreuid		 = 70;
const int  __NR_setregid		 = 71;
const int  __NR_sigsuspend		 = 72;
const int  __NR_sigpending		 = 73;
const int  __NR_sethostname	 = 74;
const int  __NR_setrlimit		 = 75;
const int  __NR_getrlimit		 = 76;	/* Back compatible = 2;Gig limited rlimit */
const int  __NR_getrusage		 = 77;
const int  __NR_gettimeofday	 = 78;
const int  __NR_settimeofday	 = 79;
const int  __NR_getgroups		 = 80;
const int  __NR_setgroups		 = 81;
const int  __NR_select		 = 82;
const int  __NR_symlink		 = 83;
const int  __NR_oldlstat		 = 84;
const int  __NR_readlink		 = 85;
const int  __NR_uselib		 = 86;
const int  __NR_swapon		 = 87;
const int  __NR_reboot		 = 88;
const int  __NR_readdir		 = 89;
const int  __NR_mmap		 = 90;
const int  __NR_munmap		 = 91;
const int  __NR_truncate		 = 92;
const int  __NR_ftruncate		 = 93;
const int  __NR_fchmod		 = 94;
const int  __NR_fchown		 = 95;
const int  __NR_getpriority	 = 96;
const int  __NR_setpriority	 = 97;
const int  __NR_profil		 = 98;
const int  __NR_statfs		 = 99;
const int  __NR_fstatfs	 =	100;
const int  __NR_ioperm	 =	101;
const int  __NR_socketcall	 =	102;
const int  __NR_syslog	 =	103;
const int  __NR_setitimer	 =	104;
const int  __NR_getitimer	 =	105;
const int  __NR_stat	 =	106;
const int  __NR_lstat	 =	107;
const int  __NR_fstat	 =	108;
const int  __NR_olduname	 =	109;
const int  __NR_iopl	 =	110;
const int  __NR_vhangup	 =	111;
const int  __NR_idle	 =	112;
const int  __NR_vm86old	 =	113;
const int  __NR_wait4	 =	114;
const int  __NR_swapoff	 =	115;
const int  __NR_sysinfo	 =	116;
const int  __NR_ipc	 =	117;
const int  __NR_fsync	 =	118;
const int  __NR_sigreturn	 =	119;
const int  __NR_clone	 =	120;
const int  __NR_setdomainname =	121;
const int  __NR_uname	 =	122;
const int  __NR_modify_ldt	 =	123;
const int  __NR_adjtimex	 =	124;
const int  __NR_mprotect	 =	125;
const int  __NR_sigprocmask =	126;
const int  __NR_create_module =	127;
const int  __NR_init_module =	128;
const int  __NR_delete_module =	129;
const int  __NR_get_kernel_syms =	130;
const int  __NR_quotactl	 =	131;
const int  __NR_getpgid	 =	132;
const int  __NR_fchdir	 =	133;
const int  __NR_bdflush	 =	134;
const int  __NR_sysfs	 =	135;
const int  __NR_personality =	136;
const int  __NR_afs_syscall =	137; /* Syscall for Andrew File System */
const int  __NR_setfsuid	 =	138;
const int  __NR_setfsgid	 =	139;
const int  __NR__llseek	 =	140;
const int  __NR_getdents	 =	141;
const int  __NR__newselect	 =	142;
const int  __NR_flock	 =	143;
const int  __NR_msync	 =	144;
const int  __NR_readv	 =	145;
const int  __NR_writev	 =	146;
const int  __NR_getsid	 =	147;
const int  __NR_fdatasync	 =	148;
const int  __NR__sysctl	 =	149;
const int  __NR_mlock	 =	150;
const int  __NR_munlock	 =	151;
const int  __NR_mlockall	 =	152;
const int  __NR_munlockall	 =	153;
const int  __NR_sched_setparam	 =	154;
const int  __NR_sched_getparam	 =	155;
const int  __NR_sched_setscheduler	 =	156;
const int  __NR_sched_getscheduler	 =	157;
const int  __NR_sched_yield	 =	158;
const int  __NR_sched_get_priority_max =	159;
const int  __NR_sched_get_priority_min =	160;
const int  __NR_sched_rr_get_interval =	161;
const int  __NR_nanosleep	 =	162;
const int  __NR_mremap	 =	163;
const int  __NR_setresuid	 =	164;
const int  __NR_getresuid	 =	165;
const int  __NR_vm86	 =	166;
const int  __NR_query_module =	167;
const int  __NR_poll	 =	168;
const int  __NR_nfsservctl	 =	169;
const int  __NR_setresgid	 =	170;
const int  __NR_getresgid	 =	171;
const int  __NR_prctl              = 172;
const int  __NR_rt_sigreturn =	173;
const int  __NR_rt_sigaction =	174;
const int  __NR_rt_sigprocmask =	175;
const int  __NR_rt_sigpending =	176;
const int  __NR_rt_sigtimedwait =	177;
const int  __NR_rt_sigqueueinfo =	178;
const int  __NR_rt_sigsuspend =	179;
const int  __NR_pread	 =	180;
const int  __NR_pwrite	 =	181;
const int  __NR_chown	 =	182;
const int  __NR_getcwd	 =	183;
const int  __NR_capget	 =	184;
const int  __NR_capset	 =	185;
const int  __NR_sigaltstack =	186;
const int  __NR_sendfile	 =	187;
const int  __NR_getpmsg	 =	188;	/* some people actually want streams */
const int  __NR_putpmsg	 =	189;	/* some people actually want streams */
const int  __NR_vfork	 =	190;
const int  __NR_ugetrlimit	 =	191;	/* SuS compliant getrlimit */
const int  __NR_mmap2	 =	192;
const int  __NR_truncate64	 =	193;
const int  __NR_ftruncate64 =	194;
const int  __NR_stat64	 =	195;
const int  __NR_lstat64	 =	196;
const int  __NR_fstat64	 =	197;
const int  __NR_lchown32	 =	198;
const int  __NR_getuid32	 =	199;
const int  __NR_getgid32	 =	200;
const int  __NR_geteuid32	 =	201;
const int  __NR_getegid32	 =	202;
const int  __NR_setreuid32	 =	203;
const int  __NR_setregid32	 =	204;
const int  __NR_getgroups32 =	205;
const int  __NR_setgroups32 =	206;
const int  __NR_fchown32	 =	207;
const int  __NR_setresuid32 =	208;
const int  __NR_getresuid32 =	209;
const int  __NR_setresgid32 =	210;
const int  __NR_getresgid32 =	211;
const int  __NR_chown32	 =	212;
const int  __NR_setuid32	 =	213;
const int  __NR_setgid32	 =	214;
const int  __NR_setfsuid32	 =	215;
const int  __NR_setfsgid32	 =	216;
const int  __NR_pivot_root	 =	217;
const int  __NR_mincore	 =	218;
const int  __NR_madvise	 =	219;
const int  __NR_madvise1	 =	219;	/* delete when C lib stub is removed */
const int  __NR_getdents64	 =	220;
const int  __NR_fcntl64	 =	221;
const int  __NR_security	 =	223;	/* syscall for security modules */
const int  __NR_gettid	 =	224;
const int  __NR_readahead	 =	225;
const int  __NR_setxattr	 =	226;
const int  __NR_lsetxattr	 =	227;
const int  __NR_fsetxattr	 =	228;
const int  __NR_getxattr	 =	229;
const int  __NR_lgetxattr	 =	230;
const int  __NR_fgetxattr	 =	231;
const int  __NR_listxattr	 =	232;
const int  __NR_llistxattr	 =	233;
const int  __NR_flistxattr	 =	234;
const int  __NR_removexattr =	235;
const int  __NR_lremovexattr =	236;
const int  __NR_fremovexattr =	237;
const int  __NR_tkill	 =	238;
const int  __NR_sendfile64	 =	239;
const int  __NR_futex	 =	240;
const int  __NR_sched_setaffinity =	241;
const int  __NR_sched_getaffinity =	242;
const int  __NR_set_thread_area =	243;
const int  __NR_get_thread_area =	244;
const int  __NR_io_setup	 =	245;
const int  __NR_io_destroy	 =	246;
const int  __NR_io_getevents =	247;
const int  __NR_io_submit	 =	248;
const int  __NR_io_cancel	 =	249;
const int  __NR_alloc_hugepages =	250;
const int  __NR_free_hugepages =	251;
const int  __NR_exit_group	 =	252;
	//Added when moving from 2.4 to 2.6.35 kernel
	//....
const int  __NR_lookup_dcookie	=	253;
const int  __NR_epoll_create	=	254;
const int  __NR_epoll_ctl		=	255;
const int  __NR_epoll_wait		=	256;
const int  __NR_remap_file_pages	=	257;
const int  __NR_set_tid_address	=	258;
const int  __NR_timer_create	=	259;
const int  __NR_timer_settime	=	(__NR_timer_create+1);
const int  __NR_timer_gettime	=	(__NR_timer_create+2);
const int  __NR_timer_getoverrun	=	(__NR_timer_create+3);
const int  __NR_timer_delete	=	(__NR_timer_create+4);
const int  __NR_clock_settime	=	(__NR_timer_create+5);
const int  __NR_clock_gettime	=	(__NR_timer_create+6);
const int  __NR_clock_getres	=	(__NR_timer_create+7);
const int  __NR_clock_nanosleep	=	(__NR_timer_create+8);
const int  __NR_statfs64		=	268;
const int  __NR_fstatfs64		=	269;
const int  __NR_tgkill		=	270;
const int  __NR_utimes		=	271;
const int  __NR_fadvise64_64	=	272;
const int  __NR_vserver		=	273;
const int  __NR_mbind		=	274;
const int  __NR_get_mempolicy	=	275;
const int  __NR_set_mempolicy	=	276;
const int  __NR_mq_open 		=	277;
const int  __NR_mq_unlink		=	(__NR_mq_open+1);
const int  __NR_mq_timedsend	=	(__NR_mq_open+2);
const int  __NR_mq_timedreceive	=	(__NR_mq_open+3);
const int  __NR_mq_notify		=	(__NR_mq_open+4);
const int  __NR_mq_getsetattr	=	(__NR_mq_open+5);
const int  __NR_kexec_load		=	283;
const int  __NR_waitid		=	284;
/* const int  __NR_sys_setaltroot	=	285; */
const int  __NR_add_key		=	286;
const int  __NR_request_key	=	287;
const int  __NR_keyctl		=	288;
const int  __NR_ioprio_set		=	289;
const int  __NR_ioprio_get		=	290;
const int  __NR_inotify_init	=	291;
const int  __NR_inotify_add_watch	=	292;
const int  __NR_inotify_rm_watch	=	293;
const int  __NR_migrate_pages	=	294;
const int  __NR_openat		=	295;
const int  __NR_mkdirat		=	296;
const int  __NR_mknodat		=	297;
const int  __NR_fchownat		=	298;
const int  __NR_futimesat		=	299;
const int  __NR_fstatat64		=	300;
const int  __NR_unlinkat		=	301;
const int  __NR_renameat		=	302;
const int  __NR_linkat		=	303;
const int  __NR_symlinkat		=	304;
const int  __NR_readlinkat		=	305;
const int  __NR_fchmodat		=	306;
const int  __NR_faccessat		=	307;
const int  __NR_pselect6		=	308;
const int  __NR_ppoll		=	309;
const int  __NR_unshare		=	310;
const int  __NR_set_robust_list	=	311;
const int  __NR_get_robust_list	=	312;
const int  __NR_splice		=	313;
const int  __NR_sync_file_range	=	314;
const int  __NR_tee		=	315;
const int  __NR_vmsplice		=	316;
const int  __NR_move_pages		=	317;
const int  __NR_getcpu		=	318;
const int  __NR_epoll_pwait	=	319;
const int  __NR_utimensat		=	320;
const int  __NR_signalfd		=	321;
const int  __NR_timerfd_create	=	322;
const int  __NR_eventfd		=	323;
const int  __NR_fallocate		=	324;
const int  __NR_timerfd_settime	=	325;
const int  __NR_timerfd_gettime	=	326;
const int  __NR_signalfd4		=	327;
const int  __NR_eventfd2		=	328;
const int  __NR_epoll_create1	=	329;
const int  __NR_dup3		=	330;
const int  __NR_pipe2		=	331;
const int  __NR_inotify_init1	=	332;
const int  __NR_preadv		=	333;
const int  __NR_pwritev		=	334;
const int  __NR_rt_tgsigqueueinfo	=	335;
const int  __NR_perf_event_open	=	336;
const int  __NR_recvmmsg		=	337;


const int  F_OK   = 0;
const int  R_OK   = 4;
const int  W_OK   = 2;
const int  X_OK   = 1;

const int _SEEK_SET      =  0;
const int _SEEK_CUR      =  1;
const int _SEEK_END      =  2;


const int  PROT_READ      = 0x1;             /* page can be read */
const int  PROT_WRITE     = 0x2;             /* page can be written */
const int  PROT_EXEC      = 0x4;             /* page can be executed */
const int  PROT_NONE      = 0x0;             /* page can not be accessed */

const int  MAP_SHARED     = 0x01;            /* Share changes */
const int  MAP_PRIVATE    = 0x02;            /* Changes are private */
const int  MAP_TYPE       = 0x0f;            /* Mask for type of mapping */
const int  MAP_FIXED      = 0x10;            /* Interpret addr exactly */
const int  MAP_ANONYMOUS  = 0x20;            /* don't use a file */

const int  MAP_GROWSDOWN  = 0x0100;          /* stack-like segment */
const int  MAP_DENYWRITE  = 0x0800;          /* ETXTBSY */
const int  MAP_EXECUTABLE = 0x1000;          /* mark it as an executable */
const int  MAP_LOCKED     = 0x2000;          /* pages are locked */
const int  MAP_NORESERVE  = 0x4000;          /* don't check for reservations */

const int  MS_ASYNC       = 1;               /* sync memory asynchronously */
const int  MS_INVALIDATE  = 2;               /* invalidate the caches */
const int  MS_SYNC        = 4;               /* synchronous memory sync */

const int  MCL_CURRENT    = 1;               /* lock all current mappings */
const int  MCL_FUTURE     = 2;               /* lock all future mappings */

const int  MADV_NORMAL    = 0x0;             /* default page-in behavior */
const int  MADV_RANDOM    = 0x1;             /* page-in minimum required */
const int  MADV_SEQUENTIAL= 0x2;             /* read-ahead aggressively */
const int  MADV_WILLNEED  = 0x3;             /* pre-fault pages */
const int  MADV_DONTNEED  = 0x4;             /* discard these pages */

/* compatibility flags */
const int  MAP_ANON       = MAP_ANONYMOUS;
const int  MAP_FILE       = 0;


const int  RUSAGE_SELF    = 0;
const int  RUSAGE_CHILDREN= (-1);
const int  RUSAGE_BOTH    = (-2) ;           /* sys_wait4() uses this */

const int  PRIO_MIN       = (-20);
const int  PRIO_MAX       = 20;

const int  PRIO_PROCESS   = 0;
const int  PRIO_PGRP      = 1;
const int  PRIO_USER      = 2;

const int  WNOHANG        = 0x00000001;
const int  WUNTRACED      = 0x00000002;

const int  __WNOTHREAD    = 0x20000000;      /* Don't wait on children of other threads in this group */
const int  __WALL         = 0x40000000;      /* Wait on all children, regardless of type */
const int  __WCLONE       = 0x80000000;      /* Wait only on non-SIGCHLD children */


const int  RLIMIT_CPU     = 0 ;              /* CPU time in ms */
const int  RLIMIT_FSIZE   = 1 ;              /* Maximum filesize */
const int  RLIMIT_DATA    = 2 ;              /* max data size */
const int  RLIMIT_STACK   = 3 ;              /* max stack size */
const int  RLIMIT_CORE    = 4 ;              /* max core file size */
const int  RLIMIT_RSS     = 5 ;              /* max resident set size */
const int  RLIMIT_NPROC   = 6 ;              /* max number of processes */
const int  RLIMIT_NOFILE  = 7 ;              /* max number of open files */
const int  RLIMIT_MEMLOCK = 8 ;              /* max locked-in-memory address space */
const int  RLIMIT_AS      = 9 ;              /* address space limit */
const int  RLIMIT_LOCKS   = 10;              /* maximum file locks held */

const int  RLIM_NLIMITS   = 11;

const int  RLIM_INFINITY  = (~0UL);


const int  LINUX_REBOOT_MAGIC1    = 0xfee1dead;
const int  LINUX_REBOOT_MAGIC2    = 672274793;
const int  LINUX_REBOOT_MAGIC2A   = 85072278;
const int  LINUX_REBOOT_MAGIC2B   = 369367448;

const int  LINUX_REBOOT_CMD_RESTART       = 0x01234567;
const int  LINUX_REBOOT_CMD_HALT          = 0xCDEF0123;
const int  LINUX_REBOOT_CMD_CAD_ON        = 0x89ABCDEF;
const int  LINUX_REBOOT_CMD_CAD_OFF       = 0x00000000;
const int  LINUX_REBOOT_CMD_POWER_OFF     = 0x4321FEDC;
const int  LINUX_REBOOT_CMD_RESTART2      = 0xA1B2C3D4;


const int  PTRACE_TRACEME         =    0;
const int  PTRACE_PEEKTEXT        =    1;
const int  PTRACE_PEEKDATA        =    2;
const int  PTRACE_PEEKUSR         =    3;
const int  PTRACE_POKETEXT        =    4;
const int  PTRACE_POKEDATA        =    5;
const int  PTRACE_POKEUSR         =    6;
const int  PTRACE_CONT            =    7;
const int  PTRACE_KILL            =    8;
const int  PTRACE_SINGLESTEP      =    9;

const int  PTRACE_ATTACH          = 0x10;
const int  PTRACE_DETACH          = 0x11;

const int  PTRACE_SYSCALL         =   24;


const int  SYS_SOCKET		=1	;	/* sys_socket(2)		*/
const int  SYS_BIND			=2	;	/* sys_bind(2)			*/
const int  SYS_CONNECT		=3	;	/* sys_connect(2)		*/
const int  SYS_LISTEN		=4	;	/* sys_listen(2)		*/
const int  SYS_ACCEPT		=5	;	/* sys_accept(2)		*/
const int  SYS_GETSOCKNAME	=6	;	/* sys_getsockname(2)		*/
const int  SYS_GETPEERNAME	=7	;	/* sys_getpeername(2)		*/
const int  SYS_SOCKETPAIR	=8	;	/* sys_socketpair(2)		*/
const int  SYS_SEND			=9	;	/* sys_send(2)			*/
const int  SYS_RECV			=10	;	/* sys_recv(2)			*/
const int  SYS_SENDTO		=11	;	/* sys_sendto(2)		*/
const int  SYS_RECVFROM		=12	;	/* sys_recvfrom(2)		*/
const int  SYS_SHUTDOWN		=13	;	/* sys_shutdown(2)		*/
const int  SYS_SETSOCKOPT	=14	;	/* sys_setsockopt(2)		*/
const int  SYS_GETSOCKOPT	=15	;	/* sys_getsockopt(2)		*/
const int  SYS_SENDMSG		=16	;	/* sys_sendmsg(2)		*/
const int  SYS_RECVMSG		=17	;	/* sys_recvmsg(2)		*/

typedef enum {
	SS_FREE = 0,			/* not allocated		*/
	SS_UNCONNECTED,			/* unconnected to any socket	*/
	SS_CONNECTING,			/* in process of connecting	*/
	SS_CONNECTED,			/* connected to socket		*/
	SS_DISCONNECTING		/* in process of disconnecting	*/
} socket_state;

#define __SO_ACCEPTCON	(1 << 16)	/* performed a listen		*/

const int  SOCK_ASYNC_NOSPACE	=0;
const int  SOCK_ASYNC_WAITDATA	=1;
const int  SOCK_NOSPACE			=2;

enum sock_type {
	_SOCK_STREAM = 1,
	_SOCK_DGRAM	= 2,
	_SOCK_RAW	= 3,
	_SOCK_RDM	= 4,
	_SOCK_SEQPACKET	= 5,
	_SOCK_PACKET	= 10
};


const int CLOCK_REALTIME           = 0;
const int CLOCK_MONOTONIC          = 1;
const int CLOCK_PROCESS_CPUTIME_ID = 2;
const int CLOCK_THREAD_CPUTIME_ID  = 3;
const int CLOCK_MONOTONIC_RAW      = 4;
const int CLOCK_REALTIME_COARSE    = 5;
const int CLOCK_MONOTONIC_COARSE   = 6;

};

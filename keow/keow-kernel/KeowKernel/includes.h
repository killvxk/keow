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

// common includes

// include windows first so we catch #define redefinitions
// in linux includes and then we can fix them

//windows stuff
#include "WinFiles.h"


//linux stuff
#include "linux_includes.h"

//safe _after_ the linux stuff (else strcpy etc in linux includes won't compile)
#include <strsafe.h>



//STL
//We'd like to use STL, but it causes lots of std c stuff to be included
//that conflicts with the linux stuff (eg. errno.h)
//So we let Keow use the Win32 API and define our own helper classes
//that don't cause a conflict

#include "List.h"
#include "String.h"


//stub info
#include "SysCallDll.h"


//keow stuff - only the generic ones here (eg Filesystem, not FilesystemKeow)

#include "Utils.h"
#include "SysCalls.h"
#include "ConstantMapping.h"
#include "Path.h"
#include "IOHandler.h"
#include "Filesystem.h"
#include "Device.h"
#include "Process.h"
#include "MountPoint.h"
#include "KernelTable.h"
#include "MemoryHelper.h"


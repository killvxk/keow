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

// ConstantMapping.h: interface for the ConstantMapping class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONSTANTMAPPING_H__ADA6F5DB_E98D_4F0F_B686_3288741C964F__INCLUDED_)
#define AFX_CONSTANTMAPPING_H__ADA6F5DB_E98D_4F0F_B686_3288741C964F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

DWORD ElfProtectionToWin32Protection(linux::Elf32_Word prot);
int Win32ErrToUnixError(DWORD err);


#endif // !defined(AFX_CONSTANTMAPPING_H__ADA6F5DB_E98D_4F0F_B686_3288741C964F__INCLUDED_)

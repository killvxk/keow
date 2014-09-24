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


#include <windows.h>


//////////////////////////////////////////////////////////////////////////////////////


void StubMain()
{
	//the keow/unix files have already been loaded by the time we get here,
	//so it is safe to load our larger dll of functions now

	OutputDebugString("Stub starting\n");

	LoadLibrary("KeowUserSysCalls.Dll");

	//should never get here (the library takes over)
	OutputDebugString("Stub failure?\n");
	ExitProcess(-1);
}

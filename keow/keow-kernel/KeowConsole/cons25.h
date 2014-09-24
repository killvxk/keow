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

// cons25.h: interface for the cons25 class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_CONS25_H__83201F53_6952_4939_A0CC_E24395AC9400__INCLUDED_)
#define AFX_CONS25_H__83201F53_6952_4939_A0CC_E24395AC9400__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//BSD Console terminal
//#	Reconstructed via infocmp from file: /usr/share/terminfo/a/ansi80x25
//cons25|ansis|ansi80x25|freebsd console (25-line ansi mode), 

class cons25
{
public:
	cons25();
	virtual ~cons25();

	void OutputChar(char c);
	void InputChar();

protected:

	int m_InputState;
	int m_OutputState;
	BYTE m_InputStateData[100];
	BYTE m_OutputStateData[100];
};

#endif // !defined(AFX_CONS25_H__83201F53_6952_4939_A0CC_E24395AC9400__INCLUDED_)

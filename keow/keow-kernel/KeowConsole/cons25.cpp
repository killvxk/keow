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

// cons25.cpp: implementation of the cons25 class.
//
//////////////////////////////////////////////////////////////////////

#include "includes.h"
#include "cons25.h"


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


//based on FOREGROUND_* BACKGROUND_* COMMON_LVB_*
#define FOREGROUND_MASK 0x000F
#define BACKGROUND_MASK 0x00F0
#define COMMON_LVB_MASK 0xFF00


//////////////////////////////////////////////////////////////////////

cons25::cons25()
{
	m_InputState=0;
	m_OutputState=0;
}

cons25::~cons25()
{
}

void cons25::OutputChar(char c)
{
	DWORD dwWritten;
	CONSOLE_SCREEN_BUFFER_INFO info;
	COORD pos;
	int firstY, screenWidth, screenHeight;

	//some characters are always special
	switch(c)
	{
	case CTRL('G'):
	case BACKSPACE:
	case TAB:
	case CR:
		WriteConsole(g_hConsoleOutput, &c, 1, &dwWritten, NULL);
		return;

	case LF:
	case FF:
	case VT:
		c=LF;
		WriteConsole(g_hConsoleOutput, &c, 1, &dwWritten, NULL);
		return;

	}

	//state machine
	switch(m_OutputState)
	{
	case 0: //no special state
		if(c==ESC)
		{
			m_OutputState = ESC;
		}
		else
		{
			switch(c)
			{
			case LF:
			case FF:
			case VT:
				c=LF;
				WriteConsole(g_hConsoleOutput, &c, 1, &dwWritten, NULL);
				break;
			default:
				WriteConsole(g_hConsoleOutput, &c, 1, &dwWritten, NULL);
				break;
			}
		}
		break;

	case ESC:
		switch(c)
		{
		case '[':
			m_OutputState = ESC<<8 | '[';
			memset(m_OutputStateData, 0, sizeof(m_OutputStateData));
			break;
		default:
			//unknown value - end esc sequence
			ktrace("Implement console code: ESC %c\n", c);
			m_OutputState = 0;
			break;
		}
		break;

	case ESC<<8 | '[':
		//lots of sequences want this, so do it first
		GetConsoleScreenBufferInfo(g_hConsoleOutput, &info);
		screenWidth = info.srWindow.Right - info.srWindow.Left + 1;
		screenHeight = info.srWindow.Bottom - info.srWindow.Top + 1;
		firstY = info.dwSize.Y - screenHeight - 1; //first line of actual screen (bottom-most lines of buffer)
		//ktrace("ESC [  .... %c   %d\n", c,m_OutputStateData[0]);

		switch(c)
		{
		case '=':
			m_OutputState = '['<<8 | '=';
			break;

		case '0': //collect numeric parameters
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
				int cnt = m_OutputStateData[0];
				//add this digit to what we have
				m_OutputStateData[cnt+1] *= 10;
				m_OutputStateData[cnt+1] += c-'0';
			}
			break;
		case ';': //number seperator
			++m_OutputStateData[0]; //next parameter
			break;

		case 'J': // ESC [ J
			{
				//clear to end of screen

				DWORD dwWritten;
				WORD len;

				pos = info.dwCursorPosition;
				len = screenWidth - pos.X;
				len += (info.dwSize.Y - pos.Y) * info.dwSize.X;
				FillConsoleOutputCharacter(g_hConsoleOutput, ' ', len, pos, &dwWritten);
			}
			m_OutputState = 0;
			break;
		case 'K': // ESC [ K
			{
				//clear to end of line

				DWORD dwWritten;
				WORD len;

				pos = info.dwCursorPosition;
				len = screenWidth - pos.X;
				FillConsoleOutputCharacter(g_hConsoleOutput, ' ', len, pos, &dwWritten);
			}
			m_OutputState = 0;
			break;

		case '`': // ESC [ n `
			{
				//move cursor to col n in current row
				int col = m_OutputStateData[1];

				pos.X = col-1;
				pos.Y = info.dwCursorPosition.Y;
				SetConsoleCursorPosition(g_hConsoleOutput, pos);
			}
			m_OutputState = 0;
			break;
		case 'H': // ESC [ n;n H
			{
				//move cursor to row,col
				if(m_OutputStateData[0] == 0) {
					pos.X = 0;
					pos.Y = firstY;
				} else {
					pos.X = m_OutputStateData[2] - 1;
					pos.Y = m_OutputStateData[1] - 1 + firstY;
ktrace("cursor pos %d,%d  first %d\n", pos.X, pos.Y, firstY);
				}
				SetConsoleCursorPosition(g_hConsoleOutput, pos);
			}
			m_OutputState = 0;
			break;

		case 'B': // ESC [ n B
			{
				//move cursor down
				pos.X = info.dwCursorPosition.X;
				if(m_OutputStateData[0] == 0)
					pos.Y = info.dwCursorPosition.Y + 1;
				else
					pos.Y = info.dwCursorPosition.Y + m_OutputStateData[1];
				SetConsoleCursorPosition(g_hConsoleOutput, pos);
			}
			m_OutputState = 0;
			break;
		case 'A': // ESC [ A
			{
				//move cursor up
				pos.X = info.dwCursorPosition.X;
				if(m_OutputStateData[0] == 0)
					pos.Y = info.dwCursorPosition.Y - 1;
				else
					pos.Y = info.dwCursorPosition.Y - m_OutputStateData[1];
				SetConsoleCursorPosition(g_hConsoleOutput, pos);
			}
			m_OutputState = 0;
			break;
		case 'C': // ESC [ C
			{
				//move cursor right
				if(m_OutputStateData[0] == 0)
					pos.X = info.dwCursorPosition.X + 1;
				else
					pos.X = info.dwCursorPosition.X + m_OutputStateData[1];
				pos.Y = info.dwCursorPosition.Y;
				SetConsoleCursorPosition(g_hConsoleOutput, pos);
			}
			m_OutputState = 0;
			break;
		case 'D': // ESC [ D
			{
				//move cursor left
				if(m_OutputStateData[0] == 0)
					pos.X = info.dwCursorPosition.X - 1;
				else
					pos.X = info.dwCursorPosition.X - m_OutputStateData[1];
				pos.Y = info.dwCursorPosition.Y;
				SetConsoleCursorPosition(g_hConsoleOutput, pos);
			}
			m_OutputState = 0;
			break;

		case 'E': // ESC [ E
			{
				//newline
				WriteConsole(g_hConsoleOutput, "\r\n", 2, &dwWritten, NULL);
			}
			m_OutputState = 0;
			break;

		case 'P': // ESC [ P
			{
				//delete character at current pos

				//move line contents left
				SMALL_RECT moveRect;
				CHAR_INFO fill;
				moveRect.Left=info.dwCursorPosition.X + 1;
				moveRect.Right=info.dwSize.X;
				moveRect.Top=info.dwCursorPosition.Y;
				moveRect.Bottom=info.dwCursorPosition.Y;
				pos.X = info.dwCursorPosition.X;
				pos.Y = moveRect.Top;
				fill.Attributes=info.wAttributes;
				fill.Char.AsciiChar=' '; //space
				ScrollConsoleScreenBuffer(g_hConsoleOutput, &moveRect, NULL, pos, &fill);
			}
			m_OutputState = 0;
			break;
		case 'M': // ESC [ M
			{
				//delete line at current pos

				//move lines up
				SMALL_RECT moveRect;
				CHAR_INFO fill;
				moveRect.Left=0;
				moveRect.Right=info.dwSize.X;
				moveRect.Top=info.dwCursorPosition.Y + 1;
				moveRect.Bottom=info.dwSize.Y;
				pos.X = 0;
				pos.Y = info.dwCursorPosition.Y;
				fill.Attributes=info.wAttributes;
				fill.Char.AsciiChar=' '; //space
				ScrollConsoleScreenBuffer(g_hConsoleOutput, &moveRect, NULL, pos, &fill);
			}
			m_OutputState = 0;
			break;

		case '@': // ESC [ @
			{
				//insert character at current pos

				//move line contents right
				SMALL_RECT moveRect;
				CHAR_INFO fill;
				moveRect.Left=info.dwCursorPosition.X;
				moveRect.Right=info.dwSize.X;
				moveRect.Top=info.dwCursorPosition.Y;
				moveRect.Bottom=info.dwCursorPosition.Y;
				pos.X = info.dwCursorPosition.X + 1;
				pos.Y = moveRect.Top;
				fill.Attributes=info.wAttributes;
				fill.Char.AsciiChar=' '; //space
				ScrollConsoleScreenBuffer(g_hConsoleOutput, &moveRect, NULL, pos, &fill);
			}
			m_OutputState = 0;
			break;
		case 'L': // ESC [ L
			{
				//insert line at current pos

				//move lines down
				SMALL_RECT moveRect;
				CHAR_INFO fill;
				moveRect.Left=0;
				moveRect.Right=info.dwSize.X;
				moveRect.Top=info.dwCursorPosition.Y;
				moveRect.Bottom=info.dwSize.Y;
				pos.X = 0;
				pos.Y = info.dwCursorPosition.Y + 1;
				fill.Attributes=info.wAttributes;
				fill.Char.AsciiChar=' '; //space
				ScrollConsoleScreenBuffer(g_hConsoleOutput, &moveRect, NULL, pos, &fill);
			}
			m_OutputState = 0;
			break;

		case 'm': // ESC [ n m
			{
				//set attributes
				int attr = m_OutputStateData[1];
				switch(attr)
				{
				case 0: //     reset all attributes to their defaults
					//white text, black background
					info.wAttributes &= ~FOREGROUND_MASK;
					info.wAttributes |= FOREGROUND_RED|FOREGROUND_BLUE|FOREGROUND_GREEN;
					break;

				case 1: //     set bold
					info.wAttributes |= FOREGROUND_INTENSITY;
					break;

				case 2: //     set half-bright (simulated with color on a color display)
					info.wAttributes &= ~FOREGROUND_INTENSITY;
					break;

				case 5: //     set blink
					break;

				case 7: //     set reverse video
					info.wAttributes |= COMMON_LVB_REVERSE_VIDEO;
					break;

				}

				SetConsoleTextAttribute(g_hConsoleOutput, info.wAttributes);
			}
			m_OutputState = 0;
			break;

		default:
			// ESC [ n;n;n; X (unknown value) - end esc sequence
			ktrace("Implement console code: ESC [ n %c\n", c);
			m_OutputState = 0;
			break;
		}
		break;

	case '['<<8 | '=':  // ESC [ = 
		switch(c)
		{
		case '0': //collect numeric parameters
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			{
				int cnt = m_OutputStateData[0];
				//add this digit to what we have
				m_OutputStateData[cnt+1] *= 10;
				m_OutputStateData[cnt+1] += c-'0';
			}
			break;
		case ';': //number seperator
			++m_OutputStateData[0];
			break;

		case 'C': // ESC [ = n C
			{
				int n = m_OutputStateData[1];

				CONSOLE_CURSOR_INFO cinfo;
				GetConsoleCursorInfo(g_hConsoleOutput, &cinfo);

				switch(n)
				{
				case 0:	//cursor normal
					cinfo.bVisible = 1;
					break;
				case 1:	//cursor visible  -where is hide?
					cinfo.bVisible = 1;
					break;
				}

				SetConsoleCursorInfo(g_hConsoleOutput, &cinfo);
			}
			m_OutputState = 0;
			break;

		default:
			// ESC [ = n;n;n; X (unknown value) - end esc sequence
			ktrace("Implement console code: ESC [ = n %c\n", c);
			m_OutputState = 0;
			break;
		}
		break;
	}
}

void cons25::InputChar()
{
	INPUT_RECORD buf;
	DWORD dwRead, dwWritten;

	char SendBuf[10]; // eg ESC [ X
	char NumToSend = 0;


	if(!ReadConsoleInput(g_hConsoleInput, &buf, 1, &dwRead))
		return;
	if(dwRead==0)
		return;

	//debug
	if(buf.EventType==KEY_EVENT) {
		ktrace("KEY_EVENT bKeyDown %d, vkey %d, ascii %d, ctrlkeys %d\n", buf.Event.KeyEvent.bKeyDown, buf.Event.KeyEvent.wVirtualKeyCode, buf.Event.KeyEvent.uChar.AsciiChar, buf.Event.KeyEvent.dwControlKeyState);
	}

	//Ctrl-C is a special case
	if(buf.EventType==KEY_EVENT
	&& buf.Event.KeyEvent.uChar.AsciiChar==CTRL('C'))
	{
		//Ctrl-C does not come as a bKeydown so handle it specially
		ktrace("Ctrl-C\n");
		SendBuf[0] = CTRL('C');
		NumToSend = 1;
	}

	if(buf.EventType==KEY_EVENT
	&& buf.Event.KeyEvent.bKeyDown!=0 )
	{
		//some keys require a different code than NT uses, translate them

		switch(buf.Event.KeyEvent.wVirtualKeyCode)
		{
		case VK_RETURN:
			SendBuf[0] = NL;
			NumToSend = 1;
			break;

		case VK_UP: // ESC [ A
			SendBuf[0] = ESC;
			SendBuf[1] = '[';
			SendBuf[2] = 'A';
			NumToSend = 3;
			break;
		case VK_DOWN: // ESC [ B
			SendBuf[0] = ESC;
			SendBuf[1] = '[';
			SendBuf[2] = 'B';
			NumToSend = 3;
			break;
		case VK_LEFT: // ESC [ D
			SendBuf[0] = ESC;
			SendBuf[1] = '[';
			SendBuf[2] = 'D';
			NumToSend = 3;
			break;
		case VK_RIGHT: // ESC [ C
			SendBuf[0] = ESC;
			SendBuf[1] = '[';
			SendBuf[2] = 'C';
			NumToSend = 3;
			break;

		case VK_HOME: // ESC [ H
			SendBuf[0] = ESC;
			SendBuf[1] = '[';
			SendBuf[2] = 'H';
			NumToSend = 3;
			break;
		case VK_END: // ESC [ F
			SendBuf[0] = ESC;
			SendBuf[1] = '[';
			SendBuf[2] = 'F';
			NumToSend = 3;
			break;

		case VK_NEXT: // ESC [ G
			SendBuf[0] = ESC;
			SendBuf[1] = '[';
			SendBuf[2] = 'G';
			NumToSend = 3;
			break;
		case VK_PRIOR: // ESC [ I
			SendBuf[0] = ESC;
			SendBuf[1] = '[';
			SendBuf[2] = 'I';
			NumToSend = 3;
			break;

		case VK_DELETE: // \177
			SendBuf[0] = '\177';
			NumToSend = 1;
			break;
		case VK_INSERT: // ESC [ L
			SendBuf[0] = ESC;
			SendBuf[1] = '[';
			SendBuf[2] = 'L';
			NumToSend = 3;
			break;

		case VK_F1: // ESC [ M
		case VK_F2: // ESC [ N
		case VK_F3: // ESC [ O
		case VK_F4: // ESC [ P
		case VK_F5: // ESC [ Q
		case VK_F6: // ESC [ R
		case VK_F7: // ESC [ S
		case VK_F8: // ESC [ T
		case VK_F9: // ESC [ U
		case VK_F10: // ESC [ V
		case VK_F11: // ESC [ W
		case VK_F12: // ESC [ X
		case VK_F13: // ESC [ Y
		case VK_F14: // ESC [ Z
		case VK_F15: // ESC [ a
		case VK_F16: // ESC [ b
		case VK_F17: // ESC [ c
		case VK_F18: // ESC [ d
		case VK_F19: // ESC [ e
		case VK_F20: // ESC [ f
		case VK_F21: // ESC [ g
		case VK_F22: // ESC [ h
		case VK_F23: // ESC [ i
		case VK_F24: // ESC [ j
			{
				SendBuf[0] = ESC;
				SendBuf[1] = '[';

				int fn = buf.Event.KeyEvent.wVirtualKeyCode - VK_F1;
				SendBuf[2] = "-MNOPQRSTUVWXYZabcdefghij"[fn];

				NumToSend = 3;
			}
			break;

		//TODO: more special key codes


		default: //this bit handles 'normal' keys that generate characters
			if(buf.Event.KeyEvent.uChar.AsciiChar==0) {
				//we read something not suitable for returning
				return;
			} else {
				SendBuf[0] = buf.Event.KeyEvent.uChar.AsciiChar;
				NumToSend = 1;
				ktrace("key %d %c\n", SendBuf[0],SendBuf[0]);
			}
			break;
		}
	}

	//send it
	WriteFile(g_hKernelTextInput, SendBuf, NumToSend, &dwWritten, 0);
	//kernel needs it NOW
	FlushFileBuffers(g_hKernelTextInput);
}


/*
Copyright (C) 1994-1995  Apogee Software, Ltd.
Copyright (C) 2002-2015  icculus.org, GNU/Linux port
Copyright (C) 2017-2018  Steven LeVesque

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef __MYPRINT_H
#define __MYPRINT_H

enum COLORS
   {
   BLACK, BLUE, GREEN, CYAN, RED, MAGENTA, BROWN, LIGHTGRAY, DARKGRAY,
   LIGHTBLUE, LIGHTGREEN, LIGHTCYAN, LIGHTRED, LIGHTMAGENTA, YELLOW, WHITE
   };

#define NONE         -1
#define SINGLE_FRAME -1
#define DOUBLE_FRAME -2

void DrawText( int x, int y, int ch, int foreground, int background );
void TextBox( int x1, int y1, int x2, int y2, int ch, int foreground, int background );
void TextFrame( int x1, int y1, int x2, int y2, int type, int foreground, int background );
void mysetxy( int x, int y );
void myputch( char ch );
int  printstring( char *string );
int  printnum( int number );
int  printunsigned( unsigned long number, int radix );
int  myprintf( char *fmt, ... );

#endif

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
#ifndef __DEBUGIO_H
#define __DEBUGIO_H

void DB_SetXY( int x, int y );
void DB_PutChar( char ch );
int  DB_PrintString( char *string );
int  DB_PrintNum( int number );
int  DB_PrintUnsigned( unsigned long number, int radix );
int  DB_printf( char *fmt, ... );

#endif

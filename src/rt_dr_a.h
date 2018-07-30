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
#ifndef _rt_dr_a_public
#define _rt_dr_a_public

//***************************************************************************
//
//    RT_DR_A.ASM - Low level draw stuff, DrawPost
//
//***************************************************************************

void SetMode240(void);
void RefreshClear(void);
void DrawPost (int height, char * column, char * buf);
void  DrawHeightPost (int height, byte * src, byte * buf); // IN rt_dr_a.asm
void R_DrawWallColumn (byte * buf);
void  DrawMenuPost (int height, byte * src, byte * buf); // IN rt_dr_a.asm
void  DrawMapPost (int height, byte * src, byte * buf); // IN rt_dr_a.asm

#endif

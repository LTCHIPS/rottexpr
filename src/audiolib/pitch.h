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
/**********************************************************************
   module: PITCH.H

   author: James R. Dose
   date:   June 14, 1994

   Public header for PITCH.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __PITCH_H
#define __PITCH_H

enum PITCH_ERRORS
   {
   PITCH_Warning = -2,
   PITCH_Error = -1,
   PITCH_Ok = 0,
   };

//void          PITCH_Init( void );
unsigned long PITCH_GetScale( int pitchoffset );
void          PITCH_UnlockMemory( void );
int           PITCH_LockMemory( void );
#endif

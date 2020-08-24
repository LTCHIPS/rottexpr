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
//****************************************************************************
//
// Private header for RT_IN.C
//
//****************************************************************************

#ifndef _rt_in_private
#define _rt_in_private


//****************************************************************************
//
// TYPEDEFS
//
//****************************************************************************


//****************************************************************************
//
// DEFINES
//
//****************************************************************************

#define  KeyInt         9  // The keyboard ISR number
#define  MReset         0
#define  MButtons       3
#define  MDelta         11
#define  MouseInt       0x33
#define  JoyScaleMax    32768
#define  JoyScaleShift  8
#define  MaxJoyValue    5000

void Mouse (int x);

#endif

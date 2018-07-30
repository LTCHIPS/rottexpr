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
#ifndef _rt_rand_public
#define _rt_rand_public

#include "develop.h"

void  InitializeRNG ( void );
int   GetRandomSeed ( void );

int   GameRNG ( void );
#define GameRandomNumber(string,val)   GameRNG()

int   RNG ( void );
#define RandomNumber(string,val)   RNG()

void  SetRNGindex ( int i );
int   GetRNGindex ( void );
#endif

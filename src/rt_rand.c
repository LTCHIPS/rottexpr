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
#include "rt_def.h"
#include "_rt_rand.h"
#include "rt_rand.h"
#include "develop.h"
#include "rt_util.h"
#include <time.h>


//****************************************************************************
//
// GLOBALS
//
//****************************************************************************

static int rndindex;
static int sndindex = 0;


//****************************************************************************
//
// GetRandomSeed ()
//
//****************************************************************************

int GetRandomSeed ( void )
{
    return ( time (NULL) % (SIZE_OF_RANDOM_TABLE) );
}

//****************************************************************************
//
// InitializeRNG ()
//
//****************************************************************************

void  InitializeRNG ( void )
{
    SetRNGindex(GetRandomSeed());
    sndindex=GetRandomSeed();
}

//****************************************************************************
//
// SetRNGindex ()
//
//****************************************************************************

void  SetRNGindex ( int i )
{
    rndindex=i;
    SoftError("RNG index set at %d\n",i);
}

//****************************************************************************
//
// GetRNGindex ()
//
//****************************************************************************

int GetRNGindex ( void )
{
    return rndindex;
}


//****************************************************************************
//
// int GameRNG (void)
//
//****************************************************************************
int   GameRNG ( void )
{
    rndindex = (rndindex+1)&(SIZE_OF_RANDOM_TABLE-1);

    return RandomTable[rndindex];
}



//****************************************************************************
//
// int RNG (void)
//
//****************************************************************************

int   RNG( void )
{
    sndindex = (sndindex+1)&(SIZE_OF_RANDOM_TABLE-1);

    return RandomTable[sndindex];
}


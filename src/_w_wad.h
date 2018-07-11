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
#ifndef _w_wad_private
#define _w_wad_private

#include "develop.h"

#define CHECKPERIOD 20


#if ( SHAREWARE == 1 )

#if ( DELUXE == 1)
#define WADCHECKSUM (54748)
#elif ( LOWCOST == 1)
#define WADCHECKSUM (12185)
#else
#define WADCHECKSUM (20567)
#endif

#else

#define WADCHECKSUM (24222)

#endif

//===============
//   TYPES
//===============


typedef struct
{
    char            name[8];
    int             handle,position,size;
    int             byteswapped;
} lumpinfo_t;


typedef struct
{
    char            identification[4];              // should be IWAD
    int             numlumps;
    int             infotableofs;
} wadinfo_t;


typedef struct
{
    int             filepos;
    int             size;
    char            name[8];
} filelump_t;

#endif

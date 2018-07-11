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
//***************************************************************************
//
//    W_WAD.C - Wad managment utilities
//
//***************************************************************************

#ifndef _w_wad_public
#define _w_wad_public

#include "byteordr.h"

void    W_InitMultipleFiles (char **filenames); // Initialize multiple wads
void    W_InitFile (char *filename);            // Init a single wad file

int     W_CheckNumForName (char *name);         // Check to see if the named lump exists
int     W_GetNumForName (char *name);           // Get the number for the named lump
char *  W_GetNameForNum (int i);                // Get the name for a number

int     W_NumLumps (void);                      // Get the current number of lumps managed
int     W_LumpLength (int lump);                // Get the length of the numbered lump
void    W_ReadLump (int lump, void *dest);      // Read the numbered lump into a buffer
void W_WriteLump (int lump, void *src);

void    *W_CacheLumpNum (int lump, int tag, converter_t converter, int numrecs);
// Cache in the numbered lump with the appropriate memory tag
void    *W_CacheLumpName (char *name, int tag, converter_t converter, int numrecs);
// Cache in the named lump with the appropriate memory tag

extern int             numlumps;
extern void            **lumpcache;

#endif

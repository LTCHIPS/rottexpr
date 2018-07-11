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
// scriplib.h

#ifndef _scriplib_public
#define _scriplib_public

#define	MAXTOKEN	128

extern	char	token[MAXTOKEN];
extern	char	name[MAXTOKEN*2];
extern	char	*scriptbuffer,*script_p,*scriptend_p;
extern	int		scriptline;
extern	boolean	endofscript;
extern   boolean tokenready;      // only true if UnGetToken was just called


void LoadScriptFile (char *filename);
void GetToken (boolean crossline);
void GetTokenEOL (boolean crossline);
void UnGetToken (void);
boolean TokenAvailable (void);

#endif

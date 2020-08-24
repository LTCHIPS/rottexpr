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
   module: USER.C

   author: James R. Dose
   date:   April 26, 1994

   Routines to parse command line options.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#include <string.h>
#include "user.h"

#define TRUE  ( 1 == 1 )
#define FALSE ( !TRUE )

/*---------------------------------------------------------------------
   Function: USER_CheckParameter

   Checks if the specified string is present in the command line.
---------------------------------------------------------------------*/

int USER_CheckParameter
   (
   const char *parameter
   )

   {
   return FALSE;
   }


/*---------------------------------------------------------------------
   Function: USER_GetText

   Checks if the specified string is present in the command line
   and returns a pointer to the text following it.
---------------------------------------------------------------------*/

char *USER_GetText
   (
   const char *parameter
   )

   {
   return NULL;
   }

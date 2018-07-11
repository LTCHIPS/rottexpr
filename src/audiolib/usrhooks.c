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
   module: USRHOOKS.C

   author: James R. Dose
   date:   July 26, 1994

   This module contains cover functions for operations the library
   needs that may be restricted by the calling program.  This code
   is left public for you to modify.
**********************************************************************/

#include <stdlib.h>
#include "usrhooks.h"


/*---------------------------------------------------------------------
   Function: USRHOOKS_GetMem

   Allocates the requested amount of memory and returns a pointer to
   its location, or NULL if an error occurs.  NOTE: pointer is assumed
   to be dword aligned.
---------------------------------------------------------------------*/

int USRHOOKS_GetMem
   (
   void **ptr,
   unsigned long size
   )

   {
   void *memory;

   memory = malloc( size );
   if ( memory == NULL )
      {
      return( USRHOOKS_Error );
      }

   *ptr = memory;

   return( USRHOOKS_Ok );
   }


/*---------------------------------------------------------------------
   Function: USRHOOKS_FreeMem

   Deallocates the memory associated with the specified pointer.
---------------------------------------------------------------------*/

int USRHOOKS_FreeMem
   (
   void *ptr
   )

   {
   if ( ptr == NULL )
      {
      return( USRHOOKS_Error );
      }

   free( ptr );

   return( USRHOOKS_Ok );
   }

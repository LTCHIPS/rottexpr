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
   module: STANDARD.H

   author: James R. Dose
   date:   May 25, 1994

   Header containing standard definitions.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __STANDARD_H
#define __STANDARD_H

typedef int boolean;
typedef int errorcode;

#ifndef TRUE
   #define TRUE ( 1 == 1 )
   #define FALSE ( !TRUE )
#endif

enum STANDARD_ERRORS
   {
   Warning = -2,
   FatalError = -1,
   Success = 0
   };

#define BITSET( data, bit ) \
   ( ( ( data ) & ( bit ) ) == ( bit ) )

#define ARRAY_LENGTH( array ) \
   ( sizeof( array ) / sizeof( ( array )[ 0 ] ) )

#define WITHIN_BOUNDS( array, index ) \
   ( ( 0 <= ( index ) ) && ( ( index ) < ARRAY_LENGTH( array ) ) )

#define FOREVER    for( ; ; )

#ifdef NDEBUG
   #define DEBUGGING 0
#else
   #define DEBUGGING 1
#endif

#define DEBUG_CODE \
   if ( DEBUGGING == 0 ) \
      { \
      } \
   else

#endif

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
   module: IRQ.H

   author: James R. Dose
   date:   August 8, 1994

   Public header for IRQ.C

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __IRQ_H
#define __IRQ_H

enum IRQ_ERRORS
   {
   IRQ_Warning = -2,
   IRQ_Error = -1,
   IRQ_Ok = 0,
   };

#define VALID_IRQ( irq )  ( ( ( irq ) >= 0 ) && ( ( irq ) <= 15 ) )

int IRQ_SetVector
   (
   int vector,
   void ( __interrupt *function )( void )
   );
int IRQ_RestoreVector
   (
   int vector
   );


#endif

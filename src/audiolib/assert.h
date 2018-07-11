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
#ifndef    __ASSERT_H

    #define    __ASSERT_H

    #ifdef NDEBUG

        #define ASSERT(f)

    #else

        #pragma aux _Assert aborts;          /* _Assert will not return */
        extern void _Assert( char *strFile, unsigned  uLine ); /*prototype */

        #define ASSERT(f)          \
            if (f)                 \
                ;                  \
            else                   \
                _Assert( __FILE__, __LINE__ )

    #endif

#else

    #error Multiple definition of ASSERT()

#endif

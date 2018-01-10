/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
//***************************************************************************
//
//    MODEXLIB.C - various utils palette funcs and modex stuff
//
//***************************************************************************

#ifndef _modexlib_public
#define _modexlib_public

#include "WinRott.h"
#include "rt_def.h"
/*
int iGLOBAL_SCREENWIDTH;//bna val 800
int iGLOBAL_SCREENHEIGHT;//bna val 600


#define MAXSCREENHEIGHT    600//     200*2
#define MAXSCREENWIDTH     800//     320*2
#define SCREENBWIDE        800*(96/320)//     96*2
#define MAXVIEWWIDTH       800//     320*2
#define SCREENWIDTH        800*(96/320)//     96*2              // default screen width in bytes
*/
//***************************************************************************
//
//    Video (ModeX) Constants
//
//***************************************************************************

#ifdef DOS
#define SC_INDEX                0x3C4
#define SC_DATA                 0x3C5
#define SC_RESET                0
#define SC_CLOCK                1
#define SC_MAPMASK              2
#define SC_CHARMAP              3
#define SC_MEMMODE              4

#define CRTC_INDEX              0x3D4
#define CRTC_DATA               0x3D5
#define CRTC_H_TOTAL            0
#define CRTC_H_DISPEND          1
#define CRTC_H_BLANK            2
#define CRTC_H_ENDBLANK         3
#define CRTC_H_RETRACE          4
#define CRTC_H_ENDRETRACE       5
#define CRTC_V_TOTAL            6
#define CRTC_OVERFLOW           7
#define CRTC_ROWSCAN            8
#define CRTC_MAXSCANLINE        9
#define CRTC_CURSORSTART        10
#define CRTC_CURSOREND          11
#define CRTC_STARTHIGH          12
#define CRTC_STARTLOW           13
#define CRTC_CURSORHIGH         14
#define CRTC_CURSORLOW          15
#define CRTC_V_RETRACE          16
#define CRTC_V_ENDRETRACE       17
#define CRTC_V_DISPEND          18
#define CRTC_OFFSET             19
#define CRTC_UNDERLINE          20
#define CRTC_V_BLANK            21
#define CRTC_V_ENDBLANK         22
#define CRTC_MODE               23
#define CRTC_LINECOMPARE        24

#define GC_INDEX                0x3CE
#define GC_DATA                 0x3CF
#define GC_SETRESET             0
#define GC_ENABLESETRESET       1
#define GC_COLORCOMPARE         2
#define GC_DATAROTATE           3
#define GC_READMAP              4
#define GC_MODE                 5
#define GC_MISCELLANEOUS        6
#define GC_COLORDONTCARE        7
#define GC_BITMASK              8

#define ATR_INDEX               0x3c0
#define ATR_MODE                16
#define ATR_OVERSCAN            17
#define ATR_COLORPLANEENABLE      18
#define ATR_PELPAN              19
#define ATR_COLORSELECT         20

#define STATUS_REGISTER_1       0x3da

#define PEL_WRITE_ADR           0x3c8
#define PEL_READ_ADR            0x3c7
#define PEL_DATA                0x3c9
#endif

extern  boolean StretchScreen;//bn�++

//extern  int      ylookup[MAXSCREENHEIGHT];      // Table of row offsets
extern  int      ylookup[1080];      // just set to max res
extern  int      linewidth;
extern  byte    *page1start;
extern  byte    *page2start;
extern  byte    *page3start;
extern  int      screensize;
extern  byte    *bufferofs;
extern  byte    *displayofs;
extern  boolean  graphicsmode;


void  GraphicsMode ( void );
void  SetTextMode ( void );
void  VL_SetVGAPlaneMode ( void );
void  VL_ClearBuffer (byte *buf, byte color);
void  VL_ClearVideo (byte color);
void  VL_DePlaneVGA (void);
void  VL_CopyDisplayToHidden ( void );
void  VL_CopyBufferToAll ( byte *buffer );
void  VL_CopyPlanarPage ( byte * src, byte * dest );
void  VL_CopyPlanarPageToMemory ( byte * src, byte * dest );
void  XFlipPage ( void );
void  WaitVBL( void );
void  TurnOffTextCursor ( void );

#ifdef __WATCOMC__
#pragma aux VGAWRITEMAP =      \
        "mov    eax,01H"       \
        "mov    edx,03c5h"     \
        "shl    eax,cl"        \
        "out    dx,al"         \
        parm    [ecx]          \
        modify exact [eax edx]

#pragma aux VGAMAPMASK =       \
        "mov    edx,03c5h"     \
        "out    dx,al"         \
        parm    [eax]          \
        modify exact [edx]

#pragma aux VGAREADMAP =      \
        "shl    eax,08H"      \
        "mov    edx,03ceh"    \
        "add    eax,04H"      \
        "out    dx,ax"        \
        parm    [eax]         \
        modify exact [eax edx]
#endif

#ifdef DOS
void  VGAMAPMASK (int x);
void  VGAREADMAP (int x);
void  VGAWRITEMAP(int x);
#else
#define VGAMAPMASK(a)
#define VGAREADMAP(a)
#define VGAWRITEMAP(a)
#endif

#endif

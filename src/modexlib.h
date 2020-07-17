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
//    MODEXLIB.C - various utils palette funcs and modex stuff
//
//***************************************************************************

#ifndef _modexlib_public
#define _modexlib_public

#include "WinRott.h"
#include "rt_def.h"
#include "SDL2/SDL.h"
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

extern  boolean StretchScreen;//bn�++

//extern  int      ylookup[MAXSCREENHEIGHT];      // Table of row offsets
extern  int      ylookup[MAXSCREENHEIGHT];      // just set to max res
extern  int      linewidth;
extern  byte    *page1start;
extern  byte    *page2start;
extern  byte    *page3start;
extern  int      screensize;
extern  byte    *bufferofs;
extern  byte    *displayofs;
extern  boolean  graphicsmode;

//extern SDL_Window* window;

extern SDL_Surface * sdl_surface;


void  GraphicsMode ( void );
void  ChangeResolution( int, int);
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
void DoScreenRotateScale(int w, int h, SDL_Texture * tex, float angle, float scale);
void  WaitVBL( void );
void  TurnOffTextCursor ( void );
void EnableHudStretch(void);
void DisableHudStretch(void);
void sdl_handle_window_events(void);

#endif

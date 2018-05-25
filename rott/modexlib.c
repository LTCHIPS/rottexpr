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

#include <stdarg.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <SDL2/SDL_video.h>
#include "modexlib.h"
//MED
#include "memcheck.h"
#include "rt_util.h"
#include "rt_net.h" // for GamePaused
#include "myprint.h"
#include "rt_view.h"
#include "queue.h"
#include "lumpy.h"
//#include "SDL2/SDL2_rotozoom.h"
//#include <SDL2/SDL_image.h>


static void StretchMemPicture ();
// GLOBAL VARIABLES

boolean StretchScreen=0;//bn�++
extern boolean iG_aimCross;
extern boolean sdl_fullscreen;
extern int iG_X_center;
extern int iG_Y_center;
char 	   *iG_buf_center;


SDL_Surface *sdl_surface = NULL;

SDL_Window * window = NULL;

static SDL_Renderer * renderer = NULL;

SDL_Surface *unstretch_sdl_surface = NULL;

static SDL_Texture *sdl_texture = NULL;

SDL_Surface *temp = NULL;

//Queue *sdl_draw_obj_queue = NULL;

boolean doRescaling = false;

int    linewidth;
//int    ylookup[MAXSCREENHEIGHT];
int    ylookup[MAXSCREENHEIGHT];//just set to max res
byte  *page1start;
byte  *page2start;
byte  *page3start;
int    screensize;
byte  *bufferofs;
byte  *displayofs;
boolean graphicsmode=false;
char        *bufofsTopLimit;
char        *bufofsBottomLimit;

void DrawCenterAim ();

#ifndef STUB_FUNCTION

/* rt_def.h isn't included, so I just put this here... */
#if !defined(ANSIESC)
#define STUB_FUNCTION fprintf(stderr,"STUB: %s at " __FILE__ ", line %d, thread %d\n",__FUNCTION__,__LINE__,getpid())
#else
#define STUB_FUNCTION
#endif

#endif

/*
====================
=
= GraphicsMode
=
====================
*/
void GraphicsMode ( void )
{
    Uint32 flags = 0;

    if (SDL_InitSubSystem (SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
    {
        Error ("Could not initialize SDL\n");
    }
    
    SDL_SetRelativeMouseMode(SDL_TRUE);
    
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");
    if (sdl_fullscreen)
        flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
    
    window = SDL_CreateWindow("Rise of the Triad",
                               SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                               iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT,
                               flags);
    
    if (window == NULL)
    {
        Error ("Could not set video mode\n");
        exit(1);
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    
    sdl_texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ABGR8888,
                                    SDL_TEXTUREACCESS_STREAMING, iGLOBAL_SCREENWIDTH,
                                    iGLOBAL_SCREENHEIGHT);
    
    sdl_surface = SDL_CreateRGBSurface(0,iGLOBAL_SCREENWIDTH,iGLOBAL_SCREENHEIGHT,8,0,0,0,0);
    
    
         
    SDL_SetSurfaceRLE(sdl_surface, 1);
                                        
    SDL_RenderSetLogicalSize(renderer, iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT);
    
}

/*
====================
=
= SetTextMode
=
====================
*/
void SetTextMode ( void )
{
    if (SDL_WasInit(SDL_INIT_VIDEO) == SDL_INIT_VIDEO) {
        if (sdl_surface != NULL) {
            SDL_FreeSurface(sdl_surface);

            sdl_surface = NULL;
        }

        SDL_QuitSubSystem (SDL_INIT_VIDEO);
    }
}

/*
====================
=
= TurnOffTextCursor
=
====================
*/
void TurnOffTextCursor ( void )
{
}

/*
====================
=
= WaitVBL
=
====================
*/
void WaitVBL( void )
{
    SDL_Delay (16667/1000);
}

/*
=======================
=
= VL_SetVGAPlaneMode
=
=======================
*/

void VL_SetVGAPlaneMode ( void )
{
    int i,offset;

    GraphicsMode();

//
// set up lookup tables
//
//bna--   linewidth = 320;
    linewidth = iGLOBAL_SCREENWIDTH;

    offset = 0;

    for (i=0; i<iGLOBAL_SCREENHEIGHT; i++)
    {
        ylookup[i]=offset;
        offset += linewidth;
    }

//    screensize=MAXSCREENHEIGHT*MAXSCREENWIDTH;
    screensize=iGLOBAL_SCREENHEIGHT*iGLOBAL_SCREENWIDTH;

    page1start=sdl_surface->pixels;
    page2start=sdl_surface->pixels;
    page3start=sdl_surface->pixels;
    displayofs = page1start;
    bufferofs = page2start;

    iG_X_center = iGLOBAL_SCREENWIDTH / 2;
    iG_Y_center = (iGLOBAL_SCREENHEIGHT / 2)+10 ;//+10 = move aim down a bit

    iG_buf_center = bufferofs + (screensize/2);//(iG_Y_center*iGLOBAL_SCREENWIDTH);//+iG_X_center;

    bufofsTopLimit =  bufferofs + screensize - iGLOBAL_SCREENWIDTH;
    bufofsBottomLimit = bufferofs + iGLOBAL_SCREENWIDTH;

    // start stretched
    EnableScreenStretch();
    XFlipPage ();
}

/*
=======================
=
= VL_CopyPlanarPage
=
=======================
*/
void VL_CopyPlanarPage ( byte * src, byte * dest )
{
    memcpy(dest,src,screensize);
}

/*
=======================
=
= VL_CopyPlanarPageToMemory
=
=======================
*/
void VL_CopyPlanarPageToMemory ( byte * src, byte * dest )
{
    memcpy(dest,src,screensize);
}

/*
=======================
=
= VL_CopyBufferToAll
=
=======================
*/
void VL_CopyBufferToAll ( byte *buffer )
{
    //STUB_FUNCTION;
}

/*
=======================
=
= VL_CopyDisplayToHidden
=
=======================
*/
void VL_CopyDisplayToHidden ( void )
{
    VL_CopyBufferToAll ( displayofs );
}

/*
=================
=
= VL_ClearBuffer
=
= Fill the entire video buffer with a given color
=
=================
*/

void VL_ClearBuffer (byte *buf, byte color)
{
    memset((byte *)buf,color,screensize);
}

/*
=================
=
= VL_ClearVideo
=
= Fill the entire video buffer with a given color
=
=================
*/

void VL_ClearVideo (byte color)
{
    memset (sdl_surface->pixels, color, iGLOBAL_SCREENWIDTH*iGLOBAL_SCREENHEIGHT);
}

/*
=================
=
= VL_DePlaneVGA
=
=================
*/

void VL_DePlaneVGA (void)
{
}

void RescaleAreaOfTexture(SDL_Renderer* renderer, SDL_Texture * source, SDL_Rect src, SDL_Rect dest)
{
    SDL_Texture * sourceToResize = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, src.w, src.h);          
    SDL_SetRenderTarget(renderer, sourceToResize);
    SDL_RenderCopy(renderer, source, &src, NULL);
    // the folowing line should reset the target to default(the screen)
    SDL_SetRenderTarget(renderer, NULL);
    
    SDL_RenderCopy(renderer, sourceToResize, NULL, &dest);
    SDL_DestroyTexture(sourceToResize);
}

int hudRescaleFactor = 1;

/*
void DrawObjsInSDLQueue(SDL_Texture * tex)
{
    SDL_SetRenderTarget(renderer, tex);
    //SDL_LockTexture(tex,NULL, sdl_surface->pixels, iGLOBAL_SCREENBWIDE);
    
    while(sdl_draw_obj_queue->sizeOfQueue != 0)
    {   
        SDLDrawObj * thing = sdl_draw_obj_queue->head->data;
        
        
        SDL_Surface * tempSurf = SDL_CreateRGBSurfaceWithFormatFrom( thing->data, (int) thing->shape->width,
                                (int) thing->shape->height, 8, (int)thing->shape->width, sdl_surface->format->format);
        
        //SDL_LockSurface(tempSurf);
        
        
        //tempSurf->pixels = (byte *) &thing->data;
        
        //SDL_UnlockSurface(tempSurf);
        
        if(tempSurf == NULL)
        {
            Error("Failed to make temporary Surface when rendering things in SDL");
            exit(1);
        }
        
        SDL_Texture * tempTex = SDL_CreateTextureFromSurface(renderer, tempSurf);
        
        if(tempTex == NULL)
        {
            Error("Failed to make temporary Texture when rendering things in SDL");
            exit(1);
        }
        
        SDL_Rect newCoords = (SDL_Rect) {(iGLOBAL_SCREENWIDTH - (320* hudRescaleFactor)) >> 1, iGLOBAL_SCREENHEIGHT - 16*hudRescaleFactor, 320*hudRescaleFactor, 16*hudRescaleFactor};
        
        SDL_RenderCopy(renderer, tempTex, NULL, &newCoords);
        
        SDL_FreeSurface(tempSurf);
        
        SDL_DestroyTexture(tempTex);
        
        dequeue(sdl_draw_obj_queue, thing);
        
    
    }
    //SDL_SetRenderTarget(renderer, NULL);
    
    //SDL_RenderCopy(renderer, tex, NULL, NULL);
    
    
    //SDL_UnlockTexture(tex);
    
    SDL_SetRenderTarget(renderer, NULL);
    
    //SDL_RenderPresent(renderer);
    

}
*/




void RenderSurface(void)
{
    SDL_Texture * newTex = SDL_CreateTextureFromSurface(renderer, sdl_surface);
    
    if (newTex == NULL) 
    {
        Error("CreateTextureFromSurface failed: %s\n", SDL_GetError());
        exit(1);
    }
   
    SDL_RenderClear(renderer);
    
    SDL_RenderCopy(renderer, newTex, NULL, NULL);
    
    if (!StretchScreen && hudRescaleFactor > 1 && doRescaling)
    {
        if(SHOW_TOP_STATUS_BAR())
            RescaleAreaOfTexture(renderer, newTex, (SDL_Rect) {(iGLOBAL_SCREENWIDTH - 320) >> 1, 0, 320, 16}, 
                   (SDL_Rect) {(iGLOBAL_SCREENWIDTH - (320 * hudRescaleFactor)) >> 1, 0, 320*hudRescaleFactor, 16*hudRescaleFactor}); //Status Bar
        if(SHOW_BOTTOM_STATUS_BAR())
            RescaleAreaOfTexture(renderer, newTex,(SDL_Rect) {(iGLOBAL_SCREENWIDTH - 320) >> 1, iGLOBAL_SCREENHEIGHT - 16, 320, 16},
               (SDL_Rect) {(iGLOBAL_SCREENWIDTH - (320* hudRescaleFactor)) >> 1, iGLOBAL_SCREENHEIGHT - 16*hudRescaleFactor, 320*hudRescaleFactor, 16*hudRescaleFactor}); //Bottom Bar
                   
    }
    
    SDL_RenderPresent(renderer);
    
    SDL_DestroyTexture(newTex);

}

/* C version of rt_vh_a.asm */

void VH_UpdateScreen (void)
{

    if (StretchScreen) { //bna++
        StretchMemPicture ();
    } else {
        DrawCenterAim ();
    }
    
    RenderSurface();
    
}


/*
=================
=
= XFlipPage
=
=================
*/

void XFlipPage ( void )
{
    if (StretchScreen) { //bna++
        StretchMemPicture ();
    } else {
        DrawCenterAim ();
    }
    RenderSurface();
    
}

void EnableScreenStretch(void)
{
    int i,offset;

    if (iGLOBAL_SCREENWIDTH <= 320 || StretchScreen) return;

    if (unstretch_sdl_surface == NULL)
    {
        /* should really be just 320x200, but there is code all over the
           places which crashes then */
        unstretch_sdl_surface = SDL_CreateRGBSurface(SDL_SWSURFACE,
                                iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT, 8, 0, 0, 0, 0);
    }

    displayofs = unstretch_sdl_surface->pixels +
                 (displayofs - (byte *)sdl_surface->pixels);
    bufferofs  = unstretch_sdl_surface->pixels;
    page1start = unstretch_sdl_surface->pixels;
    page2start = unstretch_sdl_surface->pixels;
    page3start = unstretch_sdl_surface->pixels;
    StretchScreen = 1;
}

void DisableScreenStretch(void)
{
    if (iGLOBAL_SCREENWIDTH <= 320 || !StretchScreen) return;

    displayofs = sdl_surface->pixels +
                 (displayofs - (byte *)unstretch_sdl_surface->pixels);
    bufferofs  = sdl_surface->pixels;
    page1start = sdl_surface->pixels;
    page2start = sdl_surface->pixels;
    page3start = sdl_surface->pixels;
    StretchScreen = 0;
    SDL_RenderSetLogicalSize(renderer, iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT);
    //SDL_RenderSetLogicalSize(renderer, 320, 200);
    
}

void EnableHudStretch(void)
{
    doRescaling = 1;
}

void DisableHudStretch(void)
{
    doRescaling = 0;
}

// bna section -------------------------------------------
static void StretchMemPicture ()
{
    SDL_Rect src;
    SDL_Rect dest;

    src.x = 0;
    src.y = 0;
    src.w = 320;
    src.h = 200;

    dest.x = 0;
    dest.y = 0;
    dest.w = iGLOBAL_SCREENWIDTH;
    dest.h = iGLOBAL_SCREENHEIGHT;
    SDL_SoftStretch(unstretch_sdl_surface, &src, sdl_surface, &dest);
    SDL_RenderSetLogicalSize(renderer, 320, 200); //help keep aspect ratio of menus so that the game doesn't look stretched
    
}

// bna function added start
extern	boolean ingame;
extern exit_t playstate;
int		iG_playerTilt;

void DrawCenterAim ()
{
    int x;

    int percenthealth = (locplayerstate->health * 10) / MaxHitpointsForCharacter(locplayerstate);
    int color = percenthealth < 3 ? egacolor[RED] : percenthealth < 4 ? egacolor[YELLOW] : egacolor[GREEN];

    if (iG_aimCross && !GamePaused && playstate != ex_died) {
        if (( ingame == true )&&(iGLOBAL_SCREENWIDTH>320)) {
            if ((iG_playerTilt <0 )||(iG_playerTilt >iGLOBAL_SCREENHEIGHT/2)) {
                iG_playerTilt = -(2048 - iG_playerTilt);
            }
            if (iGLOBAL_SCREENWIDTH == 640) {
                x = iG_playerTilt;
                iG_playerTilt=x/2;
            }
            iG_buf_center = bufferofs + ((iG_Y_center-iG_playerTilt)*iGLOBAL_SCREENWIDTH);//+iG_X_center;

            for (x=iG_X_center-10; x<=iG_X_center-4; x++) {
                if ((iG_buf_center+x < bufofsTopLimit)&&(iG_buf_center+x > bufofsBottomLimit)) {
                    *(iG_buf_center+x) = color;
                }
            }
            for (x=iG_X_center+4; x<=iG_X_center+10; x++) {
                if ((iG_buf_center+x < bufofsTopLimit)&&(iG_buf_center+x > bufofsBottomLimit)) {
                    *(iG_buf_center+x) = color;
                }
            }
            for (x=10; x>=4; x--) {
                if (((iG_buf_center-(x*iGLOBAL_SCREENWIDTH)+iG_X_center) < bufofsTopLimit)&&((iG_buf_center-(x*iGLOBAL_SCREENWIDTH)+iG_X_center) > bufofsBottomLimit)) {
                    *(iG_buf_center-(x*iGLOBAL_SCREENWIDTH)+iG_X_center) = color;
                }
            }
            for (x=4; x<=10; x++) {
                if (((iG_buf_center+(x*iGLOBAL_SCREENWIDTH)+iG_X_center) < bufofsTopLimit)&&((iG_buf_center+(x*iGLOBAL_SCREENWIDTH)+iG_X_center) > bufofsBottomLimit)) {
                    *(iG_buf_center+(x*iGLOBAL_SCREENWIDTH)+iG_X_center) = color;
                }
            }
        }
    }
}
// bna function added end




// bna section -------------------------------------------

void sdl_handle_window_events(void)
{
    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.window.type)
        {
            case SDL_WINDOWEVENT_FOCUS_LOST:
                SDL_SetRelativeMouseMode(SDL_FALSE);
                break;
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                SDL_SetRelativeMouseMode(SDL_TRUE);
                break;
            case SDL_WINDOWEVENT_MINIMIZED:
                SDL_SetRelativeMouseMode(SDL_FALSE);
                break;
            case SDL_WINDOWEVENT_RESTORED:
                SDL_SetRelativeMouseMode(SDL_TRUE);
                break;
            case SDL_WINDOWEVENT_CLOSE:
                event.type = SDL_QUIT;
                SDL_PushEvent(&event);
                break;
        }
    
    }

}

//extern int tics;

//void CalcTics (void);

//void DrawRotatedScreen(int cx, int cy, byte *destscreen, int angle, int scale, int masked)

void DoScreenRotateScale(int w, int h, SDL_Texture * tex, float angle, float scale)
{   
    
    SDL_RenderClear(renderer);
        
    SDL_Rect output;
    
    output.w = abs((int)((float)w * scale));
    
    output.h = abs((int)((float)h * scale));
    
    //if (output.w < MinScreenWidth)
        //output.w = MinScreenWidth;
    //if (output.h < MinScreenHeight)
        //output.h = MinScreenHeight;
               
    output.x = (iGLOBAL_SCREENWIDTH - output.w)>>1;
        
    output.y = (iGLOBAL_SCREENHEIGHT - output.h)>>1;
        
    SDL_RenderCopyEx(renderer, tex, NULL, &output, angle, NULL, SDL_FLIP_NONE);
        
    SDL_RenderPresent(renderer);


}

SDL_Texture * GetMainSurfaceAsTexture(void)
{
    return SDL_CreateTextureFromSurface(renderer, sdl_surface);
}

const SDL_Renderer * GetRenderer(void)
{
    return renderer;

}
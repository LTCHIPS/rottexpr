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
// RT_DRAW.C

#include "profile.h"
#include "rt_def.h"
#include <string.h>
#include "watcom.h"
#include "sprites.h"
#include "rt_actor.h"
#include "rt_stat.h"
#include "rt_draw.h"
#include "_rt_draw.h"
#include "rt_dr_a.h"
#include "rt_fc_a.h"
#include "rt_scale.h"
#include "rt_floor.h"
#include "rt_main.h"
#include "rt_playr.h"
#include "rt_door.h"
#include "rt_ted.h"
#include "isr.h"
#include "rt_util.h"
#include "engine.h"
#include "z_zone.h"
#include "w_wad.h"
#include "lumpy.h"
#include "rt_menu.h"
#include "rt_game.h"
#include "rt_vid.h"
#include "rt_view.h"
#include <stdio.h>
#include <stdlib.h>
#include "rt_cfg.h"
#include "rt_str.h"
#include "develop.h"
#include "rt_sound.h"
#include "rt_msg.h"
#include "modexlib.h"
#include "rt_rand.h"
#include "rt_net.h"
#include "rt_sc_a.h"


extern void VH_UpdateScreen (void);



//int testval;
/*
=============================================================================

Global Variables                                                                                                                                 GLOBAL VARIABLES

=============================================================================
*/

int iG_masked;

int whereami=-1;

byte * shadingtable;

word   tilemap[MAPSIZE][MAPSIZE]; // wall values only
byte   spotvis[MAPSIZE][MAPSIZE];
byte   mapseen[MAPSIZE][MAPSIZE];
unsigned long * lights;

int         wstart;


const int dirangle8[9] = {0,FINEANGLES/8,2*FINEANGLES/8,3*FINEANGLES/8,4*FINEANGLES/8,
                          5*FINEANGLES/8,6*FINEANGLES/8,7*FINEANGLES/8,8*FINEANGLES/8
                         };

const int dirangle16[16] = {0,FINEANGLES/16,2*FINEANGLES/16,3*FINEANGLES/16,
                            4*FINEANGLES/16,5*FINEANGLES/16,6*FINEANGLES/16,
                            7*FINEANGLES/16,8*FINEANGLES/16,9*FINEANGLES/16,
                            10*FINEANGLES/16,11*FINEANGLES/16,12*FINEANGLES/16,
                            13*FINEANGLES/16,14*FINEANGLES/16,15*FINEANGLES/16
                           };

//
// math tables
//

short   tantable[FINEANGLES];
int     sintable[FINEANGLES+FINEANGLEQUAD+1],
        *costable = sintable+(FINEANGLES/4);

//
// refresh variables
//

fixed   viewx,viewy;                                                     // the focal point
int     viewangle;
int     c_startx, c_starty;
fixed   viewsin,viewcos;
int     tics;

//
// ray tracing variables
//

long    xintercept,yintercept;

int doublestep=0;
int hp_startfrac;
int hp_srcstep;

int levelheight;

int actortime=0;
int drawtime=0;

visobj_t vislist[MAXVISIBLE],*visptr,*visstep,*farthest;

int firstcoloffset=0;

/*
==================
=
= Local Variables
=
==================
*/
static int nonbobpheight;

static visobj_t * sortedvislist[MAXVISIBLE];

static const fixed mindist = 0x1000;

static int walltime=0;

static int weaponbobx, weaponboby;

static int      pretics[3];
static int      preindex;
static int      netlump;
static int      gmasklump;

int      G_gmasklump;

static const int weaponshape[NUMWEAPGRAPHICS] =
{
#if (SHAREWARE == 0)

    W_KNIFE,
#endif

    W_MALEPISTOL1,
    W_MRIGHTPISTOL1,
    W_MP40,
    W_BAZOOKA,
    W_HEATSEEKER,
    W_DRUNK,
    W_FIREBOMB,
    W_FIREWALL,
    W_GODHAND,


#if (SHAREWARE == 0)
    W_SPLIT,
    W_KES,
    W_BAT,
    W_DOG,
    W_FEMALEPISTOL1,
    W_BMALEPISTOL1
#endif
};

void SetColorLightLevel (int x, int y, visobj_t * sprite, int dir, int color, int fullbright);
void DrawRotatedScreen(int cx, int cy, byte *destscreen, int angle, int scale, int masked);
void InterpolateMaskedWall (visobj_t * plane);
void InterpolateDoor (visobj_t * plane);
void InterpolateWall (visobj_t * plane);

/*
==================
=
= BuildTables
=
==================
*/

void BuildTables (void)
{
    byte * table;
    byte * ptr;
    int   length;
    int   i;

//
// load in tables file
//

    table=W_CacheLumpName("tables",PU_STATIC, CvtNull, 1);
    ptr=table;

//
// get size of first table
//

    memcpy(&length,ptr,sizeof(int));
    SwapIntelLong(&length);

//
// skip first table
//

    ptr+=(length+1)*sizeof(int);

//
// get size of sin/cos table
//

    memcpy(&length,ptr,sizeof(int));
    SwapIntelLong(&length);
    ptr+=sizeof(int);

//
// get sin/cos table
//
    memcpy(&sintable[0],ptr,length*sizeof(int));
    SwapIntelLongArray(&sintable[0], length);
    ptr+=(length)*sizeof(int);

//
// get size of tangent table
//

    memcpy(&length,ptr,sizeof(int));
    SwapIntelLong(&length);
    ptr+=sizeof(int);

//
// get tangent table
//
    memcpy(tantable,ptr,length*sizeof(short));
    SwapIntelShortArray(tantable, length);
    ptr+=(length)*sizeof(short);

//
// get size of gamma table
//

    memcpy(&length,ptr,sizeof(int));
    SwapIntelLong(&length);
    ptr+=sizeof(int);

//
// get gamma table
//
    memcpy(&gammatable[0],ptr,length*sizeof(byte));
    table=W_CacheLumpName("tables",PU_CACHE, CvtNull, 1);

    costable = (fixed *)&(sintable[FINEANGLES/4]);

    wstart=W_GetNumForName("WALLSTRT");
#if (SHAREWARE==0)
    netlump=W_GetNumForName("net1");
#endif
    gmasklump=W_GetNumForName("p_gmask");
    G_gmasklump = gmasklump;

    preindex=0;
    pretics[0]=0x10000;
    pretics[2]=0x10000;
    pretics[1]=0x10000;

    for(i=0; i<ANGLES; i++)
    {   angletodir[i] = (i + (ANGLES/16))/(ANGLES/8);
        if (angletodir[i] == 8)
            angletodir[i] = 0;
    }

    // Check out VENDOR.DOC file
    CheckVendor();

    if (!quiet)
        printf("RT_DRAW: Tables Initialized\n");
}


/*
========================
=
= TransformObject
=
========================
*/

boolean TransformObject (int x, int y, int *dispx, int *dispheight)
{

    fixed gx,gy,gxt,gyt,nx,ny;


//
// translate point to view centered coordinates
//
    gx = x-viewx;
    gy = y-viewy;

//
// calculate newx
//
    gxt = FixedMul(gx,viewcos);
    gyt = FixedMul(gy,viewsin);
    nx = gxt-gyt;

    if (nx<MINZ)
        return false;

    // the midpoint could put parts of the shape
    // into an adjacent wall
    //
    // calculate newy
    //
    gxt = FixedMul(gx,viewsin);
    gyt = FixedMul(gy,viewcos);
    ny = gyt+gxt;


//
// calculate perspective ratio
//

    *dispx = centerx + ny*scale/nx;            // DEBUG: use assembly divide

    *dispheight = heightnumerator/nx;

    return true;
}


/*
========================
=
= TransformPoint
=
========================
*/

void TransformPoint (int x, int y, int * screenx, int * height, int * texture, int vertical)
{

    fixed gxt,gyt,nx,ny;
    fixed gxtt,gytt;
    int gx,gy;
    int vx,vy;
    int svs,svc;


//
// translate point to view centered coordinates
//
    gx = x-viewx;
    gy = y-viewy;

//
// calculate newx
//
    gxt = FixedMul(gx,viewcos);
    gyt = FixedMul(gy,viewsin);
    nx =gxt-gyt;

    if (nx<10)
        nx=10;


//
// calculate newy
//
    gxtt = FixedMul(gx,viewsin);
    gytt = FixedMul(gy,viewcos);
    ny = gytt+gxtt;

// too close, don't overflow the divid'


    *screenx = centerx + ((ny*scale)/nx);            // DEBUG: use assembly divide

    *height = heightnumerator/nx;


    if (*screenx<0)
    {
        svc=(-centerx)*viewcos;
        svs=(-centerx)*viewsin;
        vx=(scale*viewcos)+svs;
        vy=(-scale*viewsin)+svc;
        if (vertical)
        {
            if ((viewcos-viewsin)==0)
            {
                *height=20000<<HEIGHTFRACTION;
                return;
            }
            gy=FixedScale(gx,vy,vx);
            y=gy+viewy;
            gyt = FixedMul(gy,viewsin);
            nx =gxt-gyt;
            if (nx<10)
                nx=10;
            *screenx = 0;
            *height = heightnumerator/nx;
        }
        else
        {
            if ((-viewsin-viewcos)==0)
            {
                *height=20000<<HEIGHTFRACTION;
                return;
            }
            gx=FixedScale(gy,vx,vy);
            x=gx+viewx;
            gxt = FixedMul(gx,viewcos);
            nx =gxt-gyt;
            if (nx<10)
                nx=10;
            *screenx = 0;
            *height = heightnumerator/nx;
        }
    }
    else if (*screenx>=viewwidth)
    {
        svc=(centerx)*viewcos;
        svs=(centerx)*viewsin;
        vx=(scale*viewcos)+svs;
        vy=(-scale*viewsin)+svc;
        if (vertical)
        {
            if ((viewcos+viewsin)==0)
            {
                *height=20000<<HEIGHTFRACTION;
                return;
            }
            gy=FixedScale(gx,vy,vx);
            y=gy+viewy;
            gyt = FixedMul(gy,viewsin);
            nx =gxt-gyt;
            if (nx<10)
                nx=10;
            *screenx = viewwidth-1;
            *height = heightnumerator/nx;
        }
        else
        {
            if ((-viewsin+viewcos)==0)
            {
                *height=20000<<HEIGHTFRACTION;
                return;
            }
            gx=FixedScale(gy,vx,vy);
            x=gx+viewx;
            gxt = FixedMul(gx,viewcos);
            nx =gxt-gyt;
            if (nx<10)
                nx=10;
            *screenx = viewwidth-1;
            *height = heightnumerator/nx;
        }
    }
    if (vertical)
        *texture=(y-*texture)&0xffff;
    else
        *texture=(x-*texture)&0xffff;
}

/*
========================
=
= TransformSimplePoint
=
========================
*/

boolean TransformSimplePoint (int x, int y, int * screenx, int * height, int * texture, int vertical)
{

    fixed gxt,gyt,nx,ny;
    fixed gxtt,gytt;
    int gx,gy;


//
// translate point to view centered coordinates
//
    gx = x-viewx;
    gy = y-viewy;

//
// calculate newx
//
    gxt = FixedMul(gx,viewcos);
    gyt = FixedMul(gy,viewsin);
    nx =gxt-gyt;

    if (nx<MINZ)
        return false;


//
// calculate newy
//
    gxtt = FixedMul(gx,viewsin);
    gytt = FixedMul(gy,viewcos);
    ny = gytt+gxtt;

// too close, don't overflow the divid'


    *screenx = centerx + ((ny*scale)/nx);            // DEBUG: use assembly divide

    *height = heightnumerator/nx;

    if (vertical)
        *texture=(y-*texture)&0xffff;
    else
        *texture=(x-*texture)&0xffff;

    return true;
}


/*
========================
=
= TransformPlane
=
========================
*/

boolean TransformPlane (int x1, int y1, int x2, int y2, visobj_t * plane)
{
    boolean result2;
    boolean result1;
    boolean vertical;
    int txstart,txend;

    vertical=((x2-x1)==0);
    plane->viewx=vertical;
    txstart=plane->texturestart;
    txend=plane->textureend;
    result1=TransformSimplePoint(x1,y1,&(plane->x1),&(plane->h1),&(plane->texturestart),vertical);
    result2=TransformSimplePoint(x2,y2,&(plane->x2),&(plane->h2),&(plane->textureend),vertical);
    if (result1==true)
    {
        if (plane->x1>=viewwidth)
            return false;
        if (result2==false)
        {
            plane->textureend=txend;
            TransformPoint(x2,y2,&(plane->x2),&(plane->h2),&(plane->textureend),vertical);
        }
    }
    else
    {
        if (result2==false)
            return false;
        else
        {
            if (plane->x2<0)
                return false;
            plane->texturestart=txstart;
            TransformPoint(x1,y1,&(plane->x1),&(plane->h1),&(plane->texturestart),vertical);
        }
    }
    if (plane->x1<0)
    {
        plane->texturestart=txstart;
        TransformPoint(x1,y1,&(plane->x1),&(plane->h1),&(plane->texturestart),vertical);
    }
    if (plane->x2>=viewwidth)
    {
        plane->textureend=txend;
        TransformPoint(x2,y2,&(plane->x2),&(plane->h2),&(plane->textureend),vertical);
    }

    plane->viewheight=(plane->h1+plane->h2)>>1;

    if ((plane->viewheight>=(2000<<HEIGHTFRACTION)) || (plane->x1>=viewwidth-1) || (plane->x2<=0))
        return false;

    return true;
}

//==========================================================================

/*
====================
=
= CalcHeight
=
= Calculates the height of xintercept,yintercept from viewx,viewy
=
====================
*/

int       CalcHeight (void)
{
    fixed  gxt,gyt,nx;
    long            gx,gy;

    whereami=0;

    gx = xintercept-viewx;
    gxt = FixedMul(gx,viewcos);

    gy = yintercept-viewy;
    gyt = FixedMul(gy,viewsin);

    nx = gxt-gyt;

    if (nx<mindist)
        nx=mindist; // don't let divide overflo'

    return (heightnumerator/nx);
}

//==========================================================================



/*
=====================
=
= StatRotate
=
=====================
*/

int  StatRotate (statobj_t *temp)
{
    int    angle;
    int    dx,dy;

    whereami=2;

    dx = temp->x - player->x;
    dy = player->y - temp->y;
    angle = atan2_appx(dx,dy);

    angle = angle-VANG180-dirangle8[temp->count];
    angle+=ANGLES/16;
    while (angle>=ANGLES)
        angle-=ANGLES;
    while (angle<0)
        angle+=ANGLES;

    return angle/(ANGLES/8);

}




/*
=====================
=
= CalcRotate
=
=====================
*/

int   CalcRotate (objtype *ob)
{
    int    angle,viewangle;
    int    dx,dy;
    int    rotation;

    whereami=1;

    // this isn't exactly correct, as it should vary by a trig value'
    // but it is close enough with only eight rotations
    /*
    if (ob->obclass == b_robobossobj)
    	viewangle = player->angle;
    else
    	viewangle = player->angle + (centerx - ob->viewx)/8;*/
    dx = ob->x - player->x;
    dy = player->y - ob->y;
    viewangle = atan2_appx(dx,dy);

    if ((ob->obclass >= p_bazookaobj) || (ob->obclass == missileobj))
    {   angle = viewangle - ob->angle;
    }
    else if ((ob->obclass > wallopobj) && (ob->obclass != b_darksnakeobj))
        angle =  (viewangle-ANG180)- ob->angle;
    else if (ob->state->rotate == 16)
        angle =  (viewangle-ANG180)- dirangle16[ob->dir];
    else
        angle =  (viewangle-ANG180)- dirangle8[ob->dir];

    if (ob->state->rotate == true)
        angle += ANGLES/16;
    else if (ob->state->rotate == 16)
        angle += ANGLES/32;

    while (angle>=ANGLES)
        angle-=ANGLES;
    while (angle<0)
        angle+=ANGLES;

    if (ob->state->rotate == 2)        // 2 rotation pain frame
    {   rotation = 4*(angle/(ANG180));
        return rotation;
    }

    if (ob->state->rotate == 16)
    {   rotation = angle/(ANGLES/16);
        return rotation;
    }
    rotation = angle/(ANGLES/8);
    return rotation;

}

/*
======================
=
= SortScaleds
= Sort the scaleds using a HEAPSORT
=
======================
*/

#define SGN(x)          ((x>0) ? (1) : ((x==0) ? (0) : (-1)))

/*--------------------------------------------------------------------------*/
int CompareHeights(s1p,s2p) visobj_t **s1p,**s2p;
{
    whereami=3;
    return SGN((*s1p)->viewheight-(*s2p)->viewheight);
}

void SwitchPointers(s1p,s2p) visobj_t **s1p,**s2p;
{
    visobj_t * temp;

    whereami=4;
    temp=*s1p;
    *s1p=*s2p;
    *s2p=temp;
}


void SortVisibleList( int numvisible, visobj_t * vlist )
{
    int i;

    whereami=5;
    for (i=0; i<numvisible; i++)
        sortedvislist[i]=&(vlist[i]);
    hsort((char *)&(sortedvislist[0]),numvisible,sizeof(visobj_t *),&CompareHeights,&SwitchPointers);
}

/*
=====================
=
= DrawScaleds
=
= Draws all objects that are visible
=
=====================
*/

#define HF_1 (24)
#define HF_2 (72)

void DrawScaleds (void)
{


    int   i,numvisible;
    int   gx,gy;
    byte   *visspot;
    boolean result;
    statobj_t *statptr;
    objtype   *obj;
    maskedwallobj_t* tmwall;

    whereami=6;

//
// place maskwall objects
//
    for(tmwall=FIRSTMASKEDWALL; tmwall; tmwall=tmwall->next)
    {
        if (spotvis[tmwall->tilex][tmwall->tiley])
        {
            mapseen[tmwall->tilex][tmwall->tiley]=1;
            if (tmwall->vertical)
            {
                gx=(tmwall->tilex<<16)+0x8000;
                gy=(tmwall->tiley<<16);
                visptr->texturestart=0;
                visptr->textureend=0;
                if (viewx<gx)
                    result=TransformPlane(gx,gy,gx,gy+0xffff,visptr);
                else
                    result=TransformPlane(gx,gy+0xffff,gx,gy,visptr);
                visptr->shapenum=tmwall->bottomtexture;
                visptr->altshapenum=tmwall->midtexture;
                visptr->viewx=tmwall->toptexture;
                visptr->shapesize=2;
            }
            else
            {
                gx=(tmwall->tilex<<16);
                gy=(tmwall->tiley<<16)+0x8000;
                visptr->texturestart=0;
                visptr->textureend=0;
                if (viewy<gy)
                    result=TransformPlane(gx+0xffff,gy,gx,gy,visptr);
                else
                    result=TransformPlane(gx,gy,gx+0xffff,gy,visptr);
                visptr->shapenum=tmwall->bottomtexture;
                visptr->altshapenum=tmwall->midtexture;
                visptr->viewx=tmwall->toptexture;
                visptr->shapesize=2;
            }
            if ((tmwall->flags&MW_TOPFLIPPING) &&
                    (nonbobpheight>64)
               )
            {
                visptr->viewx++;
            }
            else if ((tmwall->flags&MW_BOTTOMFLIPPING) &&
                     (nonbobpheight>maxheight-32)
                    )
            {
                visptr->shapenum++;
            }
            if ((visptr < &vislist[MAXVISIBLE-1]) && (result==true)) // don't let it overflo'
                visptr++;
        }
    }
//
// place static objects
//
    UpdateClientControls();
    for (statptr = firstactivestat ; statptr; statptr=statptr->nextactive)
    {   //redraw:
        if((visptr->shapenum = statptr->shapenum) == NOTHING)
            continue;

        visptr->shapenum += shapestart;
        if ((visptr->shapenum <= shapestart) ||
                (visptr->shapenum >= shapestop))
            Error("actor shapenum %d out of range (%d-%d)",visptr->shapenum,shapestart,shapestop);

        visspot = statptr->visspot;
        if (!((*(visspot-0)) ||
                (*(visspot-1)) ||
                (*(visspot+1)) ||
                (*(visspot-129)) ||
                (*(visspot-128)) ||
                (*(visspot-127)) ||
                (*(visspot+129)) ||
                (*(visspot+128)) ||
                (*(visspot+127))))
        {   statptr->flags &= ~FL_VISIBLE;
            continue;     // not visible
        }

        result = TransformObject (statptr->x,statptr->y,&(visptr->viewx),&(visptr->viewheight));

        if ((result==false) || (visptr->viewheight< (1<<(HEIGHTFRACTION+2))))
            continue;                         // to close to the object
        statptr->flags |= FL_SEEN;

        statptr->flags |= FL_VISIBLE;

        if (statptr->flags & FL_ROTATING)
            visptr->shapenum += StatRotate(statptr);

        if (statptr->flags&FL_TRANSLUCENT)
        {
            visptr->shapesize=1;
            if (statptr->flags&FL_FADING)
                visptr->h2=transparentlevel;
            else
                visptr->h2=FIXEDTRANSLEVEL;
            SetSpriteLightLevel(statptr->x,statptr->y,visptr,0,(statptr->flags&FL_FULLLIGHT));
        }
        else if (statptr->flags&FL_SOLIDCOLOR)
        {
            visptr->shapesize=4;
            visptr->h2=statptr->hitpoints;
        }
        else if (statptr->flags&FL_COLORED)
        {
            visptr->shapesize=0;
                SetColorLightLevel(statptr->x,statptr->y,visptr,
                                   0,statptr->hitpoints,
                                   (statptr->flags&FL_FULLLIGHT));
        }
        else
        {
            visptr->shapesize=0;
            SetSpriteLightLevel(statptr->x,statptr->y,visptr,0,(statptr->flags&FL_FULLLIGHT));
        }

        visptr->h1=pheight-statptr->z;

        if ((statptr->itemnumber != (unsigned int)-1) &&
                (statptr->flags&FL_HEIGHTFLIPPABLE)
           )
        {
            if (statptr->itemnumber==stat_disk)
            {
                int value;
                value=nonbobpheight-statptr->z-32;
                if ((value<=HF_2) && (value>HF_1))
                {
                    visptr->shapenum++;
                }
                else if ((value<=HF_1) && (value>=-HF_1))
                {
                    visptr->shapenum+=2;
                }
                else if ((value<-HF_1) && (value>=-HF_2))
                {
                    visptr->shapenum+=3;
                }
                else if (value<-HF_2)
                {
                    visptr->shapenum+=4;
                }
            }
            else if ((nonbobpheight-statptr->z)<-16)
            {
                visptr->shapenum++;
            }
        }

        if (visptr < &vislist[MAXVISIBLE-1]) // don't let it overflo'
            visptr++;


    }
//
// place active objects
//
    UpdateClientControls();
    for (obj = firstactive; obj; obj=obj->nextactive)
    {
        if (obj==player)
            continue;

        if ((visptr->shapenum = obj->shapenum) == NOTHING)
            continue;                         // no shape

        visptr->shapenum += shapestart;
        if ((visptr->shapenum <= shapestart) ||
                (visptr->shapenum >= shapestop))
            Error("actor shapenum %d out of range (%d-%d)",visptr->shapenum,shapestart,shapestop);
        visspot = &spotvis[obj->tilex][obj->tiley];

        //
        // could be in any of the nine surrounding tiles
        //
        if (*visspot
                || ( *(visspot-1))
                || ( *(visspot+1))
                || ( *(visspot-129))
                || ( *(visspot-128))
                || ( *(visspot-127))
                || ( *(visspot+129))
                || ( *(visspot+128))
                || ( *(visspot+127)) )
        {

//        result = TransformObject (obj->drawx, obj->drawy,&(visptr->viewx),&(visptr->viewheight));
            result = TransformObject (obj->x, obj->y,&(visptr->viewx),&(visptr->viewheight));
            if ((result==false) || (visptr->viewheight< (1<<(HEIGHTFRACTION+2))))
                continue;                         // to close to the object
            if (obj->state->rotate)
                visptr->shapenum += CalcRotate (obj);

            visptr->shapesize=0;

            if (player->flags&FL_SHROOMS)
            {
                visptr->shapesize=4;
                visptr->h2=(GetTicCount()&0xff);
            }
            if (obj->obclass==playerobj)
            {
                if (obj->flags&FL_GODMODE)
                {
                    visptr->shapesize=4;
                    visptr->h2=240+(GetTicCount()&0x7);
                }
                else if (obj->flags & FL_COLORED)
                {
                    playertype *pstate;

                    M_LINKSTATE(obj,pstate);
                        SetColorLightLevel(obj->x,obj->y,visptr,
                                           obj->dir,pstate->uniformcolor,
                                           (obj->flags&FL_FULLLIGHT) );
                }
                else
                    SetSpriteLightLevel(obj->x,obj->y,visptr,obj->dir,(obj->flags&FL_FULLLIGHT));

            }
            else
            {
                if ((obj->obclass >= b_darianobj) && (obj->obclass <= b_robobossobj) &&
                        MISCVARS->redindex)
                {
                    visptr->colormap=redmap+((MISCVARS->redindex-1)<<8);
                }
                else
                {
                    SetSpriteLightLevel(obj->x,obj->y,visptr,obj->dir,(obj->flags&FL_FULLLIGHT));
                }
            }

            visptr->h1= pheight - obj->z;

            if (obj->obclass==diskobj)
            {
                int value;
                value=nonbobpheight-obj->z-32;
                if ((value<=HF_2) && (value>HF_1))
                {
                    visptr->shapenum++;
                }
                else if ((value<=HF_1) && (value>=-HF_1))
                {
                    visptr->shapenum+=2;
                }
                else if ((value<-HF_1) && (value>=-HF_2))
                {
                    visptr->shapenum+=3;
                }
                else if (value<-HF_2)
                {
                    visptr->shapenum+=4;
                }
            }
            else if ( (obj->obclass==pillarobj) &&
                      ((nonbobpheight-obj->z)<-16)
                    )
            {
                visptr->shapenum++;
            }

            if (visptr < &vislist[MAXVISIBLE-1]) // don't let it overflo'
                visptr++;
            obj->flags |= FL_SEEN;
            obj->flags |= FL_VISIBLE;
        }
        else
            obj->flags &= ~FL_VISIBLE;
    }
//
// draw from back to front
//
    numvisible = visptr-&vislist[0];
    if (!numvisible)
        return;                                     // no visible objects
    SortVisibleList( numvisible, &vislist[0] );
    UpdateClientControls();
    for (i = 0; i<numvisible; i++)
    {
        //
        // draw farthest
        //

        if (sortedvislist[i]->shapesize==4) {

            ScaleSolidShape(sortedvislist[i]);

        } else if (sortedvislist[i]->shapesize==3) {

            InterpolateDoor (sortedvislist[i]);

        } else if (sortedvislist[i]->shapesize==2) {

            InterpolateMaskedWall (sortedvislist[i]);

        } else if (sortedvislist[i]->shapesize==1) {

            ScaleTransparentShape(sortedvislist[i]);

        } else {

            ScaleShape(sortedvislist[i]);

        }

    }
}

//==========================================================================





/*
==============
=
= DrawPlayerWeapon
=
= Draw the player's hand'
=
==============
*/

void DrawPlayerWeapon (void)
{
    int shapenum,index;
    int xdisp=0;
    int ydisp=0;
    int female,black;
    int altshape=0;

    whereami=7;

    SoftError("\n attackframe: %d, weaponframe: %d, weapondowntics: %d"
              " weaponuptics: %d",locplayerstate->attackframe,
              locplayerstate->weaponframe,locplayerstate->weapondowntics,
              locplayerstate->weaponuptics);

    if ((locplayerstate->NETCAPTURED == 1) && (!locplayerstate->HASKNIFE))
        return;

    if (locplayerstate->weapon != -1)
    {   female = ((locplayerstate->player == 1) || (locplayerstate->player == 3));
        black = (locplayerstate->player == 2);

        if (((locplayerstate->NETCAPTURED >= 1) || (locplayerstate->NETCAPTURED == -2)) && (locplayerstate->HASKNIFE == 1)) // if raising or lowering
        {   index = 0;
            shapenum = gunsstart + weaponshape[index] + locplayerstate->weaponframe;
        }
        else if  (locplayerstate->weapon != wp_twopistol)
        {   if (locplayerstate->weapon==wp_pistol)
            {   if (female)
                    index = NUMWEAPGRAPHICS-2;
                else if (black)
                    index = NUMWEAPGRAPHICS-1;
                else
#if (SHAREWARE == 0)
                    index = 1;
#else
                    index = 0;
#endif
            }
            else
#if (SHAREWARE == 0)

                index = locplayerstate->weapon + 1;
#else
                index = locplayerstate->weapon;
#endif

            if ((index<0) || (index>=NUMWEAPGRAPHICS))
                Error ("Weapon shapenum out of range\n");
            shapenum = gunsstart + weaponshape[index] + locplayerstate->weaponframe;

#if (SHAREWARE == 0)
            if ((shapenum < W_GetNumForName("KNIFE1")) ||
                    (shapenum > W_GetNumForName("DOGPAW4"))
               )
#else
            if ((shapenum < W_GetNumForName("MPIST11")) ||
                    (shapenum > W_GetNumForName("GODHAND8"))
               )
#endif
                Error("\n illegal weapon shapenum %d, index %d, weaponframe %d",
                      shapenum,index,locplayerstate->weaponframe);
        }

        else
        {
#if (SHAREWARE == 0)
            if (female)
            {   altshape = W_FLEFTPISTOL1;
                shapenum = W_FRIGHTPISTOL1;
            }
            else if (black)
            {   altshape = W_BMLEFTPISTOL1;
                shapenum = W_BMRIGHTPISTOL1;
            }
            else
#endif
            {   altshape = W_MLEFTPISTOL1;
                shapenum = W_MRIGHTPISTOL1;
            }

            altshape += gunsstart;
            shapenum += gunsstart;
            if (locplayerstate->weaponframe > 2)
                altshape += (locplayerstate->weaponframe - 3);
            else
                shapenum += locplayerstate->weaponframe;
        }

        if (!(locplayerstate->NETCAPTURED) ||
                (locplayerstate->NETCAPTURED == -1) ||
                (locplayerstate->HASKNIFE == 0))
        {   switch (locplayerstate->weapon)
            {

            case wp_godhand:
                break;

            case wp_mp40:
                break;

            case wp_firewall:
                ydisp = 10;
                break;

            case wp_bazooka:
                break;

            case wp_heatseeker:
                ydisp = 20;
                break;

            case wp_pistol:
                break;

            case wp_twopistol:
                xdisp = 80;
                break;

            case wp_drunk:
                ydisp = 10;
                break;

            case wp_firebomb:
                break;



#if (SHAREWARE == 0)

            case wp_kes:
                break;

            case wp_bat:
                xdisp = 20;
                break;

            case wp_split:
                ydisp = 20;
                break;


            case wp_dog:
                break;


#endif

            default:
                Error("Illegal weapon value = %d\n",locplayerstate->weapon);
                break;
            }
        }
        else
            xdisp = 60;



        //used for dual pistols?
        if (altshape)
        {
            int temp;
            int delta;

            temp = weaponscale;
            delta = FixedMul((weaponbobx<<9),weaponscale);
            weaponscale += delta;
            ScaleWeapon(xdisp - weaponbobx,ydisp + weaponboby + locplayerstate->weaponheight,shapenum);
            weaponscale -= delta;
            ScaleWeapon(weaponbobx - 80,ydisp + weaponboby + locplayerstate->weaponheight,altshape);
            weaponscale = temp;
        }
        else
        {
            int temp;
            int delta;

            temp = weaponscale;
            delta = FixedMul((weaponbobx<<9),weaponscale);
            weaponscale -= delta;
            ScaleWeapon(xdisp + weaponbobx,ydisp + weaponboby + locplayerstate->weaponheight,shapenum);
            weaponscale = temp;
        }
    }
}

void AdaptDetail ( void )
{
#if PROFILE
    return;
#else

    whereami=8;
    if ((preindex<0) || (preindex>2))
        Error("preindex out of range\n");
    pretics[preindex]=(pretics[0]+pretics[1]+pretics[2]+(tics<<16)+0x8000)>>2;
    if (pretics[preindex]>GOLOWER)
    {
        pretics[0]=GOHIGHER;
        pretics[1]=GOHIGHER;
        pretics[2]=GOHIGHER;
        doublestep++;
        if (doublestep>2) doublestep=2;
    }
    else if (pretics[preindex]<GOHIGHER)
    {
        if (doublestep>0)
            doublestep--;
    }
    preindex++;
    if (preindex>2)
        preindex=0;
#endif
}



/*
=====================
=
= CalcTics
=
=====================
*/

void CalcTics (void)
{

#if PROFILE
    tics=PROFILETICS;
    GetTicCount()+=PROFILETICS;
    oldtime=GetTicCount();
    return;
#else
    volatile int tc;

    whereami=9;
//   SoftError("InCalcTics\n");
//   SoftError("CT GetTicCount()=%ld\n",GetTicCount());
//   SoftError("CT oldtime=%ld\n",oldtime);

//
// calculate tics since last refresh for adaptive timing
//

    tc=GetTicCount();
    while (tc==oldtime) {
        tc=GetTicCount();    /* endwhile */
    }
    tics=tc-oldtime;

//   SoftError("CT GetTicCount()=%ld\n",GetTicCount());
//   if (tics>MAXTICS)
//      {
//      tc-=(tics-MAXTICS);
//      GetTicCount() = tc;
//     tics = MAXTICS;
//      }

    if (demoplayback || demorecord)
    {
        if (tics>MAXTICS)
        {
            tc=oldtime+MAXTICS;
            tics=MAXTICS;
            ISR_SetTime(tc);
        }
    }
    oldtime=tc;
#endif

}

/*
==========================
=
= SetSpriteLightLevel
=
==========================
*/

void SetSpriteLightLevel (int x, int y, visobj_t * sprite, int dir, int fullbright)
{
    int i;
    int lv;
    int intercept;

    whereami=10;

    if (MISCVARS->GASON==1)
    {
        sprite->colormap=greenmap+(MISCVARS->gasindex<<8);
        return;
    }

    if (fulllight || fullbright)
    {
        sprite->colormap=colormap+(1<<12);
        return;
    }

    if (fog)
    {
        i=((sprite->viewheight*200/iGLOBAL_SCREENHEIGHT)>>normalshade)+minshade;
        if (i>maxshade) i=maxshade;
        sprite->colormap=colormap+(i<<8);
    }
    else
    {
        if (lightsource)
        {
            if (dir==east || dir==west)
                intercept=(x>>11)&0x1c;
            else
                intercept=(y>>11)&0x1c;

            lv=(((LightSourceAt(x>>16,y>>16)>>intercept)&0xf)>>1);
            i=maxshade-(sprite->viewheight>>normalshade)-lv;
            if (i<minshade) i=minshade;
            sprite->colormap=colormap+(i<<8);
        }
        else
        {
            i=maxshade-(sprite->viewheight>>normalshade);
            if (i<minshade) i=minshade;
            sprite->colormap=colormap+(i<<8);
        }
    }
}

/*
==========================
=
= SetColorLightLevel
=
==========================
*/

void SetColorLightLevel (int x, int y, visobj_t * sprite, int dir, int color, int fullbright)
{
    int i;
    int lv;
    int intercept;
    int height;
    byte * map;


    whereami=11;
    height=sprite->viewheight<<1;
    map=playermaps[color];
    if (MISCVARS->GASON==1)
    {
        sprite->colormap=greenmap+(MISCVARS->gasindex<<8);
        return;
    }

    if ((fulllight) || (fullbright))
    {
        sprite->colormap=map+(1<<12);
        return;
    }

    if (fog)
    {
        i=((height*200/iGLOBAL_SCREENHEIGHT)>>normalshade)+minshade;
        if (i>maxshade) i=maxshade;
        sprite->colormap=map+(i<<8);
    }
    else
    {
        if (lightsource)
        {
            if (dir==east || dir==west)
                intercept=(x>>11)&0x1c;
            else
                intercept=(y>>11)&0x1c;

            lv=(((LightSourceAt(x>>16,y>>16)>>intercept)&0xf)>>1);
            i=maxshade-(height>>normalshade)-lv;
            if (i<minshade) i=minshade;
            sprite->colormap=map+(i<<8);
        }
        else
        {
            i=maxshade-(height>>normalshade);
            if (i<minshade) i=minshade;
            sprite->colormap=map+(i<<8);
        }
    }
}

/*
==========================
=
= SetWallLightLevel
=
==========================
*/

void SetWallLightLevel (wallcast_t * post)
{
    int la;
    int lv;
    int i;

    whereami=12;
    if (MISCVARS->GASON==1)
    {
        shadingtable=greenmap+(MISCVARS->gasindex<<8);
        return;
    }

    switch (post->posttype)
    {
    case 0:
        la=0;
        break;
    case 1:
        la=4;
        break;
    case 2:
        la=(4-gamestate.difficulty);
        break;
    case 3:
        la=3+(4-gamestate.difficulty);
        break;
    }

    if (lightsource)
    {
        int x,y;
        int intercept;

        x=post->offset>>7;
        y=post->offset&0x7f;
        intercept=(post->texture>>11)&0x1c;
        lv=(((LightSourceAt(x,y)>>intercept)&0xf)>>1);
    }
    else
        lv=0;
    if (fulllight)
    {
        if (fog)
        {
            i =16+minshade-lv+la;
            if (i>maxshade+la) i=maxshade+la;
            shadingtable=colormap+(i<<8);
        }
        else
        {
            i =maxshade-16-lv+la;
            if (i>=maxshade) i=maxshade;
            if (i<minshade+la) i=minshade+la;
            shadingtable=colormap+(i<<8);
        }
        return;
    }
    if (fog)
    {
        i =((post->wallheight*200/iGLOBAL_SCREENHEIGHT)>>normalshade)+minshade-lv+la;
        if (i>maxshade+la) i=maxshade+la;
        shadingtable=colormap+(i<<8);
    }
    else
    {
        i =maxshade-(post->wallheight>>normalshade)-lv+la;
        if (i>=maxshade) i=maxshade;
        if (i<minshade+la) i=minshade+la;
        shadingtable=colormap+(i<<8);
    }
}



/*
====================
=
= DrawWallPost
=
====================
*/

void DrawWallPost ( wallcast_t * post, byte * buf)
{
    int ht;
    int topscreen;
    int bottomscreen;
    byte * src;
    byte * src2;

    whereami=42;
    if (post->lump)
        src=W_CacheLumpNum(post->lump,PU_CACHE, CvtNull, 1);
    if (post->alttile!=0)
    {
        if (post->alttile==-1)
        {
            ht=maxheight+32;
            dc_invscale   = post->wallheight<<(10-HEIGHTFRACTION);
            dc_texturemid = (pheight<<SFRACBITS)+(SFRACUNIT>>1);
            topscreen     = centeryfrac - FixedMul(dc_texturemid,dc_invscale);
            bottomscreen  = topscreen + (dc_invscale*ht);
            dc_yh = ((bottomscreen-1)>>SFRACBITS)+1;
            if (dc_yh < 0)
            {
                post->floorclip=-1;
                post->ceilingclip=0;
            }
            else if (dc_yh >= viewheight)
            {
                post->floorclip=viewheight-1;
                post->ceilingclip=viewheight;
            }
            else
            {
                post->floorclip=dc_yh-1;
                post->ceilingclip=dc_yh;
            }
            return;
        }
        else
        {
            ht=nominalheight;
            src2=W_CacheLumpNum(post->alttile,PU_CACHE, CvtNull, 1);
        }
    }
    else
    {
        ht=maxheight+32;
        src2=src;
    }

    dc_invscale   = post->wallheight<<(10-HEIGHTFRACTION);
    dc_texturemid = (pheight<<SFRACBITS)+(SFRACUNIT>>1);
    topscreen     = centeryfrac - FixedMul(dc_texturemid,dc_invscale);
    bottomscreen  = topscreen + (dc_invscale*ht);
    dc_yl = (topscreen+SFRACUNIT-1)>>SFRACBITS;
    dc_yh = ((bottomscreen-1)>>SFRACBITS)+1;

    if (dc_yl >= viewheight)
    {
        post->ceilingclip=viewheight;
        post->floorclip=viewheight-1;
        return;
    }
    else if (dc_yl < 0)
        dc_yl = 0;

    dc_iscale     = (64<<(16+HEIGHTFRACTION))/post->wallheight;

    if (dc_yh < 0)
    {
        post->floorclip=-1;
        post->ceilingclip=0;
        goto bottomcheck;
    }
    else if (dc_yh > viewheight)
        dc_yh = viewheight;

    post->ceilingclip=dc_yl;
    post->floorclip=dc_yh-1;
    dc_source=src2+((post->texture>>4)&0xfc0);
    R_DrawWallColumn (buf);

bottomcheck:

    if (ht!=nominalheight)
        return;

    dc_texturemid-=(nominalheight<<SFRACBITS);
    topscreen     = centeryfrac - FixedMul(dc_texturemid,dc_invscale);
    bottomscreen  = topscreen + (dc_invscale<<6);
    dc_yl = (topscreen+SFRACUNIT-1)>>SFRACBITS;
    dc_yh = ((bottomscreen-1)>>SFRACBITS);

    if (dc_yl >= viewheight)
        return;
    else if (dc_yl < 0)
        dc_yl = 0;
    if (dc_yh < 0)
        return;
    else if (dc_yh > viewheight)
        dc_yh = viewheight;
    post->floorclip=dc_yh-1;
    dc_source=src+((post->texture>>4)&0xfc0);
    R_DrawWallColumn (buf);
}

/*
====================
=
= DrawWalls
=
====================
*/

void   DrawWalls (void)
{
    byte * buf;
    int plane;
    wallcast_t * post;

    whereami=13;

    plane = 0;

    if (doublestep>1)
    {
        {
            buf=bufferofs;
            for (post=&posts[plane]; post<&posts[viewwidth]; post+=2,buf+=2)
            {
                SetWallLightLevel(post);
                DrawWallPost(post,buf);
                DrawWallPost(post,buf+1);
                (post+1)->ceilingclip=post->ceilingclip;
                (post+1)->floorclip=post->floorclip;
            }
        }
    }
    else
    {
        {
            buf=bufferofs;
            for (post=&posts[plane]; post<&posts[viewwidth]; post++,buf++)
            {
                SetWallLightLevel(post);
                DrawWallPost(post,buf);
            }
        }
    }
}


/*
====================
=
= TransformDoors
=
====================
*/

void TransformDoors( void )
{
    int i;
    int numvisible;
    boolean result;
    int gx,gy;
    visobj_t visdoorlist[MAXVISIBLEDOORS],*doorptr;

    whereami=14;
    doorptr=&visdoorlist[0];
//
// place door objects
//

    for (i = 0; i<doornum; i++)
    {
        if (spotvis[doorobjlist[i]->tilex][doorobjlist[i]->tiley])
        {
            mapseen[doorobjlist[i]->tilex][doorobjlist[i]->tiley]=1;
            doorptr->texturestart=0;
            doorptr->textureend=0;
            if (doorobjlist[i]->vertical)
            {
                gx=(doorobjlist[i]->tilex<<16)+0x8000;
                gy=(doorobjlist[i]->tiley<<16);
                if (viewx<gx)
                    result=TransformPlane(gx,gy,gx,gy+0xffff,doorptr);
                else
                    result=TransformPlane(gx,gy+0xffff,gx,gy,doorptr);
            }
            else
            {
                gx=(doorobjlist[i]->tilex<<16);
                gy=(doorobjlist[i]->tiley<<16)+0x8000;
                if (viewy<gy)
                    result=TransformPlane(gx+0xffff,gy,gx,gy,doorptr);
                else
                    result=TransformPlane(gx,gy,gx+0xffff,gy,doorptr);
            }
            if (result==true)
            {
                doorptr->viewx=0;
                doorptr->shapenum=doorobjlist[i]->texture;
                doorptr->altshapenum=doorobjlist[i]->alttexture;
                if (doorobjlist[i]->texture==doorobjlist[i]->basetexture)
                {
                    doorptr->shapesize=(doorobjlist[i]->tilex<<7)+doorobjlist[i]->tiley;
                    if (doorptr < &visdoorlist[MAXVISIBLEDOORS-1]) // don't let it overflo'
                        doorptr++;
                }
                else
                {
                    doorptr->shapesize=3;
                    memcpy(visptr,doorptr,sizeof(visobj_t));
                    if (visptr < &vislist[MAXVISIBLE-1])
                        visptr++;
                }
            }
        }
    }
//
// draw from back to front
//
    numvisible = doorptr-&visdoorlist[0];
    if (!numvisible)
        return;
    SortVisibleList( numvisible, &visdoorlist[0] );
    for (i = 0; i<numvisible; i++)
    {
        //
        // draw farthest
        //
        InterpolateWall (sortedvislist[i]);
    }
}


/*
====================
=
= TransformPushWalls
=
====================
*/

void TransformPushWalls( void )
{
    int   i;
    int   gx,gy;
    byte   *visspot;
    visobj_t *savedptr;
    int numvisible;
    boolean result;

    whereami=15;
    savedptr=visptr;
    //
    // place pwall objects
    //
    for (i = 0; i<pwallnum; i++)
    {
        if ((pwallobjlist[i]->action==pw_pushed) || (pwallobjlist[i]->action==pw_npushed))
            continue;
        visspot = &spotvis[pwallobjlist[i]->x>>16][pwallobjlist[i]->y>>16];
        if (*visspot
                || ( *(visspot-1))
                || ( *(visspot+1))
                || ( *(visspot-128))
                || ( *(visspot+128)))
        {
            gx=pwallobjlist[i]->x;
            gy=pwallobjlist[i]->y;
            mapseen[gx>>16][gy>>16]=1;
            if (viewx<gx)
            {
                if (viewy<gy)
                {
                    visptr->texturestart=(gx-0x8000)&0xffff;
                    visptr->textureend=visptr->texturestart;
                    result=TransformPlane(gx+0x7fff,gy-0x8000,gx-0x8000,gy-0x8000,visptr);
                    visptr->texturestart^=0xffff;
                    visptr->textureend^=0xffff;
                    visptr->shapenum=pwallobjlist[i]->texture;
                    visptr->shapesize=((pwallobjlist[i]->x>>16)<<7)+(pwallobjlist[i]->y>>16);
                    visptr->viewx+=2;
                    if ((visptr < &vislist[MAXVISIBLE-1]) && (result==true)) // don't let it overflo'
                        visptr++;
                    visptr->texturestart=(gy-0x8000)&0xffff;
                    visptr->textureend=visptr->texturestart;//-0xffff;
                    result=TransformPlane(gx-0x8000,gy-0x8000,gx-0x8000,gy+0x7fff,visptr);
                }
                else
                {
                    visptr->texturestart=(gy-0x8000)&0xffff;
                    visptr->textureend=visptr->texturestart;//-0xffff;
                    result=TransformPlane(gx-0x8000,gy-0x8000,gx-0x8000,gy+0x7fff,visptr);
                    visptr->shapenum=pwallobjlist[i]->texture;
                    visptr->shapesize=((pwallobjlist[i]->x>>16)<<7)+(pwallobjlist[i]->y>>16);
                    visptr->viewx+=2;
                    if ((visptr < &vislist[MAXVISIBLE-1]) && (result==true)) // don't let it overflo'
                        visptr++;
                    visptr->texturestart=(gx-0x8000)&0xffff;
                    visptr->textureend=visptr->texturestart;//-0xffff;
                    result=TransformPlane(gx-0x8000,gy+0x7fff,gx+0x7fff,gy+0x7fff,visptr);
                }
            }
            else
            {
                if (viewy<gy)
                {
                    visptr->texturestart=(gy-0x8000)&0xffff;
                    visptr->textureend=visptr->texturestart;
                    result=TransformPlane(gx+0x7fff,gy+0x7fff,gx+0x7fff,gy-0x8000,visptr);
                    visptr->texturestart^=0xffff;
                    visptr->textureend^=0xffff;
                    visptr->shapenum=pwallobjlist[i]->texture;
                    visptr->shapesize=((pwallobjlist[i]->x>>16)<<7)+(pwallobjlist[i]->y>>16);
                    visptr->viewx+=2;
                    if ((visptr < &vislist[MAXVISIBLE-1]) && (result==true)) // don't let it overflo'
                        visptr++;
                    visptr->texturestart=(gx-0x8000)&0xffff;
                    visptr->textureend=visptr->texturestart;
                    result=TransformPlane(gx+0x7fff,gy-0x8000,gx-0x8000,gy-0x8000,visptr);
                    visptr->texturestart^=0xffff;
                    visptr->textureend^=0xffff;
                }
                else
                {
                    visptr->texturestart=(gx-0x8000)&0xffff;
                    visptr->textureend=visptr->texturestart;//-0xffff;
                    result=TransformPlane(gx-0x8000,gy+0x7fff,gx+0x7fff,gy+0x7fff,visptr);
                    visptr->shapenum=pwallobjlist[i]->texture;
                    visptr->shapesize=((pwallobjlist[i]->x>>16)<<7)+(pwallobjlist[i]->y>>16);
                    visptr->viewx+=2;
                    if ((visptr < &vislist[MAXVISIBLE-1]) && (result==true)) // don't let it overflo'
                        visptr++;
                    visptr->texturestart=(gy-0x8000)&0xffff;
                    visptr->textureend=visptr->texturestart;
                    result=TransformPlane(gx+0x7fff,gy+0x7fff,gx+0x7fff,gy-0x8000,visptr);
                    visptr->texturestart^=0xffff;
                    visptr->textureend^=0xffff;
                }
            }
            visptr->viewx+=2;
            visptr->shapenum=pwallobjlist[i]->texture;
            visptr->shapesize=((pwallobjlist[i]->x>>16)<<7)+(pwallobjlist[i]->y>>16);
            if ((visptr < &vislist[MAXVISIBLE-1]) && (result==true)) // don't let it overflo'
                visptr++;
        }
    }


//
// draw from back to front
//
    numvisible = visptr-savedptr;
    if (!numvisible)
        return;
    SortVisibleList( numvisible, savedptr );
    for (i = 0; i<numvisible; i++)
    {
        //
        // draw farthest
        //
        if (sortedvislist[i]->shapenum & 0x1000)
            sortedvislist[i]->shapenum=animwalls[sortedvislist[i]->shapenum&0x3ff].texture;
        sortedvislist[i]->altshapenum=0;
        InterpolateWall (sortedvislist[i]);
    }
    visptr=savedptr;
}

/*
====================
=
= WallRefresh
=
====================
*/

void WallRefresh (void)
{
    volatile int dtime;
    int mag;
    int yzangle;

    whereami=16;
    firstcoloffset=(firstcoloffset+(tics<<8))&65535;

    dtime=GetFastTics();
    if (missobj)
    {
        viewangle=missobj->angle;
        viewx=missobj->x-costable[viewangle];
        viewy=missobj->y+sintable[viewangle];
        pheight = missobj->z + 32;
        nonbobpheight=pheight;
        spotvis[missobj->tilex][missobj->tiley]=1;
        yzangle=missobj->yzangle;
    }
    else
    {
        if (player->flags&FL_SHROOMS)
        {
            viewangle = (player->angle + FixedMulShift(FINEANGLES,sintable[(GetTicCount()<<5)&(FINEANGLES-1)],(16+4)))&(FINEANGLES-1);
            ChangeFocalWidth(FixedMulShift(40,sintable[(GetTicCount()<<5)&(FINEANGLES-1)],16));
        }
        else
            viewangle = player->angle;
        if ((viewangle<0) && (viewangle>=FINEANGLES))
            Error ("View angle out of range = %d\n",viewangle);
        viewx = player->x;
        viewy = player->y;
        pheight = player->z + locplayerstate->playerheight + locplayerstate->heightoffset;
        nonbobpheight=pheight;
        if (
            (
                (player->z == nominalheight) ||
                (IsPlatform(player->tilex,player->tiley)) ||
                (DiskAt(player->tilex,player->tiley))
            ) &&
            (!(player->flags & FL_DOGMODE)) &&
            (BobbinOn==true) &&
            (GamePaused==false)
        )
        {
            int mag;

            mag=(player->speed>MAXBOB ? MAXBOB : player->speed);

            pheight+=FixedMulShift(mag,sintable[(GetTicCount()<<7)&2047],28);

            weaponbobx=FixedMulShift(mag,costable[((GetTicCount()<<5))&(FINEANGLES-1)],27);
            weaponboby=FixedMulShift(mag,sintable[((GetTicCount()<<5))&((FINEANGLES/2)-1)],26);
        }
        else
        {
            weaponbobx=0;
            weaponboby=0;
        }
        yzangle=player->yzangle;
        spotvis[player->tilex][player->tiley]=1;
    }

    if (yzangle > ANG180)
        pheight -= (sintable[yzangle&2047] >> 14);
    else
        pheight += (sintable[yzangle&2047] >> 14);

    viewx -= (FixedMul(sintable[yzangle&2047],costable[viewangle&2047])>>1);
    viewy += (FixedMul(sintable[yzangle&2047],sintable[viewangle&2047])>>1);

// Set YZ angle

    centery=viewheight>>1;

    if (yzangle>ANG180)
        centery-=FixedMul(FINEANGLES-yzangle,yzangleconverter);
    else
        centery+=FixedMul(yzangle,yzangleconverter);

    centeryfrac=(centery<<16);

    if (pheight < 1)
        pheight = 1;
    else if (pheight > maxheight+30)
        pheight = maxheight+30;

    if (nonbobpheight < 1)
        nonbobpheight = 1;
    else if (nonbobpheight > maxheight+30)
        nonbobpheight = maxheight+30;

    // Set light level of touchplates etc.

    mag=7+((3-gamestate.difficulty)<<2);

    transparentlevel=FixedMul(mag,sintable[(GetTicCount()<<5)&(FINEANGLES-1)])+mag;

    viewsin = sintable[viewangle];
    viewcos = costable[viewangle];
    c_startx=(scale*viewcos)-(centerx*viewsin);
    c_starty=(-scale*viewsin)-(centerx*viewcos);
    Refresh ();
    UpdateClientControls();
    TransformPushWalls();
    TransformDoors();
    UpdateClientControls();
    DrawWalls();
    UpdateClientControls();
    walltime=GetFastTics()-dtime;

}


/*
====================
=
= GetRainBoundingBox
=
====================
*/

void GetRainBoundingBox (int * xmin, int * xmax, int * ymin, int * ymax)
{
    wallcast_t * post;
    int x,y;

    // zero out all boundaries by default

    *xmax=0;
    *ymax=0;
    *xmin=127<<16;
    *ymin=127<<16;

    // check player's x and y

    if (viewx<(*xmin))
        (*xmin)=viewx;
    else if (viewx>(*xmax))
        (*xmax)=viewx;

    if (viewy<(*ymin))
        (*ymin)=viewy;
    else if (viewy>(*ymax))
        (*ymax)=viewy;

    for (post=&posts[0]; post<&posts[viewwidth]; post+=(viewwidth>>2))
    {
        x=(post->offset>>7)<<16;
        y=(post->offset&0x7f)<<16;

        if (x<(*xmin))
            (*xmin)=x;
        else if (x>(*xmax))
            (*xmax)=x;

        if (y<(*ymin))
            (*ymin)=y;
        else if (y>(*ymax))
            (*ymax)=y;
    }
}

/*
========================
=
= InterpolateWall
=
========================
*/

void InterpolateWall (visobj_t * plane)
{
    int d1,d2;
    int top;
    int topinc;
    int bot;
    int botinc;
    int i;
    int texture;
    int dh;
    int dx;
    int height;

    whereami=17;
    dx=(plane->x2-plane->x1+1);
    if (plane->h1<=0 || plane->h2<=0 || dx==0)
        return;
    d1=(1<<(16+HEIGHTFRACTION)) / plane->h1;
    d2=(1<<(16+HEIGHTFRACTION)) / plane->h2;
    dh=(((plane->h2-plane->h1)<<DHEIGHTFRACTION)+(1<<(DHEIGHTFRACTION-1)))/dx;
    top=0;
    topinc=FixedMulShift(d1,plane->textureend-plane->texturestart,4);
    bot=d2*dx;
    botinc=d1-d2;
    height=plane->h1<<DHEIGHTFRACTION;
    if (plane->x1>=viewwidth)
        return;
    for (i=plane->x1; i<=plane->x2; i++)
    {
        if ((i>=0 && i<viewwidth)&&(posts[i].wallheight<=(height>>DHEIGHTFRACTION)))
        {
            if (bot)
            {
                texture=((top/bot)+(plane->texturestart>>4))&0xfc0;
                posts[i].texture=texture<<4;
                posts[i].lump=plane->shapenum;
                posts[i].alttile=plane->altshapenum;
                posts[i].posttype=plane->viewx;
                posts[i].offset=plane->shapesize;
                posts[i].wallheight=height>>DHEIGHTFRACTION;
            }
        }
        top+=topinc;
        bot+=botinc;
        height+=dh;
    }
}


/*
========================
=
= InterpolateDoor
=
========================
*/

void InterpolateDoor (visobj_t * plane)
{
    int d1,d2;
    int top;
    int topinc;
    int bot;
    int botinc;
    int i;
    int texture;
    int dh;
    int dx;
    int height;
    int bottomscreen;
    byte * shape;
    byte * shape2;
    byte * buf;
    patch_t *p;

    whereami=18;
    dx=(plane->x2-plane->x1+1);
    if (plane->h1<=0 || plane->h2<=0 || dx==0)
        return;
    shape=W_CacheLumpNum(plane->shapenum,PU_CACHE, Cvt_patch_t, 1);
    shape2=W_CacheLumpNum(plane->altshapenum,PU_CACHE, Cvt_patch_t, 1);
    p=(patch_t *)shape;
    d1=(1<<(16+HEIGHTFRACTION)) / plane->h1;
    d2=(1<<(16+HEIGHTFRACTION)) / plane->h2;
    dh=(((plane->h2-plane->h1)<<DHEIGHTFRACTION)+(1<<(DHEIGHTFRACTION-1)))/dx;
    topinc=FixedMulShift(d1,plane->textureend-plane->texturestart,4);
    botinc=d1-d2;
    if (plane->x1>=viewwidth)
        return;
    {
        top=0;
        bot=(d2*dx);
        height=(plane->h1<<DHEIGHTFRACTION);
        buf=(byte *)bufferofs+(plane->x1);

        for (i=plane->x1; i<=plane->x2; i++,buf++)
        {
            if ((i>=0 && i<viewwidth) && (bot!=0) && (posts[i].wallheight<=(height>>DHEIGHTFRACTION)) )
            {
                dc_invscale=height>>(HEIGHTFRACTION+DHEIGHTFRACTION-10);
                dc_iscale = 0xffffffffu/(unsigned)dc_invscale;
                dc_texturemid=((pheight-nominalheight+p->topoffset)<<SFRACBITS)+(SFRACUNIT>>1);
                sprtopoffset=centeryfrac - FixedMul(dc_texturemid,dc_invscale);

                texture=((top/bot)+(plane->texturestart>>4))>>6;
                SetLightLevel(height>>DHEIGHTFRACTION);
                ScaleMaskedPost (p->collumnofs[texture]+shape,buf);

                if (levelheight>1)
                {
                    sprtopoffset-=(dc_invscale<<6)*(levelheight-1);
                    bottomscreen =sprtopoffset + (dc_invscale*nominalheight);
                    dc_yl = (sprtopoffset+SFRACUNIT-1)>>SFRACBITS;
                    dc_yh = ((bottomscreen-1)>>SFRACBITS)+1;
                    if (dc_yl >= viewheight)
                        continue;
                    else if (dc_yl < 0)
                        dc_yl = 0;
                    if (dc_yh > viewheight)
                        dc_yh = viewheight;

                    dc_source=shape2+((texture<<6)&0xfc0);
                    R_DrawWallColumn (buf);
                }
            }

            top+=topinc;
            bot+=botinc;
            height+=dh;
        }
    }
}


/*
========================
=
= InterpolateMaskedWall
=
========================
*/

void InterpolateMaskedWall (visobj_t * plane)
{
    int d1,d2;
    int top;
    int topinc;
    int bot;
    int botinc;
    int i;
    int j;
    int texture;
    int dh;
    int dx;
    int height;
    byte * shape;
    byte * shape2;
    byte * shape3;
    byte * buf;
    transpatch_t *p;
    patch_t *p2;
    patch_t *p3;
    boolean drawbottom,drawmiddle,drawtop;
    int topoffset;

    whereami=19;
    dx=(plane->x2-plane->x1+1);
    if (plane->h1<=0 || plane->h2<=0 || dx==0)
        return;
    if (plane->altshapenum>=0)
    {
        drawmiddle=true;
        shape2=W_CacheLumpNum(plane->altshapenum,PU_CACHE, Cvt_patch_t, 1);
        p2=(patch_t *)shape2;
        topoffset=p2->topoffset;
    }
    else
    {
        drawmiddle=false;
    }
    if (plane->viewx>=0)
    {
        drawtop=true;
        shape3=W_CacheLumpNum(plane->viewx,PU_CACHE, Cvt_patch_t, 1);
        p3=(patch_t *)shape3;
        topoffset=p3->topoffset;
    }
    else
    {
        drawtop=false;
    }
    if (plane->shapenum>=0)
    {
        drawbottom=true;
        shape=W_CacheLumpNum(plane->shapenum,PU_CACHE, Cvt_transpatch_t, 1);
        p = (transpatch_t *)shape;
        topoffset=p->topoffset;
    }
    else
    {
        drawbottom=false;
    }

    d1=(1<<(16+HEIGHTFRACTION)) / plane->h1;
    d2=(1<<(16+HEIGHTFRACTION)) / plane->h2;
    dh=(((plane->h2-plane->h1)<<DHEIGHTFRACTION)+(1<<(DHEIGHTFRACTION-1)))/dx;
    topinc=FixedMulShift(d1,plane->textureend-plane->texturestart,4);
    botinc=d1-d2;
    if (plane->x1>=viewwidth)
        return;
    {
        top=0;
        bot=(d2*dx);
        height=(plane->h1<<DHEIGHTFRACTION);
        buf=(byte *)bufferofs+(plane->x1);
        for (i=plane->x1; i<=plane->x2; i++,buf++)
        {
            if ((i>=0 && i<viewwidth) && (bot!=0) && (posts[i].wallheight<=(height>>DHEIGHTFRACTION)) )
            {
                dc_invscale=height>>(HEIGHTFRACTION+DHEIGHTFRACTION-10);
                dc_iscale = 0xffffffffu/(unsigned)dc_invscale;
                dc_texturemid=((pheight-nominalheight+topoffset)<<SFRACBITS)+(SFRACUNIT>>1);
                sprtopoffset=centeryfrac - FixedMul(dc_texturemid,dc_invscale);

                texture=((top/bot)+(plane->texturestart>>4))>>6;
                SetLightLevel(height>>DHEIGHTFRACTION);
                if (drawbottom==true)
                    ScaleTransparentPost (p->collumnofs[texture]+shape,buf,(p->translevel+8));
                for (j=0; j<levelheight-2; j++)
                {
                    sprtopoffset-=(dc_invscale<<6);
                    dc_texturemid+=(1<<22);
                    if (drawmiddle==true)
                        ScaleMaskedPost (p2->collumnofs[texture]+shape2,buf);
                }
                if (levelheight>1)
                {
                    sprtopoffset-=(dc_invscale<<6);
                    dc_texturemid+=(1<<22);
                    if (drawtop==true)
                        ScaleMaskedPost (p3->collumnofs[texture]+shape3,buf);
                }
            }
            top+=topinc;
            bot+=botinc;
            height+=dh;
        }
    }
}

/*
========================
=
= DrawPlayerLocation
=
========================
*/
#define PLX  (320-24)
#define PLY  16
void DrawPlayerLocation ( void )
{
    int i;
    char buf[30];

    CurrentFont=tinyfont;

    whereami=20;
    for (i=0; i<18; i++)
        memset((byte *)bufferofs+(ylookup[i+PLY])+PLX,0,6);
    px=PLX;
    py=PLY;
    VW_DrawPropString(strupr(itoa(player->x,&buf[0],16)));
    px=PLX;
    py=PLY+6;
    VW_DrawPropString(strupr(itoa(player->y,&buf[0],16)));
    px=PLX;
    py=PLY+12;
    VW_DrawPropString(strupr(itoa(player->angle,&buf[0],16)));
}



/*
========================
=
= ThreeDRefresh
=
========================
*/

//void RotateScreen(int startAngle, int endAngle, int startScale, int endScale, int time, int option, boolean fadeOut);

void RotateScreenScaleFloat(float startAngle, float endAngle, float startScale, float endScale, int time, boolean fadeOut, boolean drawPlayScreen);

int playerview=0;
void      ThreeDRefresh (void)
{
    objtype * tempptr;

    whereami=21;
    tempptr=player;

//
// Erase old messages
//

    RestoreMessageBackground();

    bufferofs += screenofs;

    RefreshClear();

    UpdateClientControls ();

//
// follow the walls from there to the right, drawwing as we go
//

    visptr = &vislist[0];
    WallRefresh ();

    UpdateClientControls ();

    if (fandc)
        DrawPlanes();

    UpdateClientControls ();

//
// draw all the scaled images
//
    DrawScaleds();                                         // draw scaled stuff

    UpdateClientControls ();

    if (!missobj)
    {
        if (locplayerstate->NETCAPTURED && (locplayerstate->NETCAPTURED != -2))
        {
            int value;

            if (locplayerstate->NETCAPTURED < 0)
                value = -locplayerstate->NETCAPTURED;
            else
                value = locplayerstate->NETCAPTURED;
            DrawScreenSizedSprite(netlump+value-1);
        }
        DrawPlayerWeapon ();    // draw player's hand'

        if (SCREENEYE)
            DrawScreenSprite(SCREENEYE->targettilex,SCREENEYE->targettiley,SCREENEYE->state->condition + GIBEYE1 + shapestart);
        UpdateClientControls ();

        if (player->flags&FL_GASMASK)
            DrawScreenSizedSprite(gmasklump);


        if ( SHOW_PLAYER_STATS() )
        {
            DrawStats ();
        }

        DoBorderShifts ();

        UpdateClientControls ();
    }

    bufferofs -= screenofs;
    DrawMessages();
    bufferofs += screenofs;

    if ( ((GamePaused==true) && (!Keyboard[sc_LShift])) ||
            (controlupdatestarted==0)
       )
        DrawPause ();

//
// show screen and time last cycle
//
    if ((fizzlein==true) && (modemgame==false))
    {
        if (newlevel==true)
            ShutdownClientControls();
        bufferofs-=screenofs;
        DrawPlayScreen (true);
        //void RotateScreen(int startAngle, int endAngle, int startScale, int endScale, int time);
        
        RotateScreenScaleFloat(0, 360, 0.01875, 1.0, (VBLCOUNTER*3)/4, false, true);

        //RotateScreen(0,FINEANGLES,FINEANGLES,FINEANGLES*8,(VBLCOUNTER*3)/4, 0, false);
        //RotateBuffer(0,FINEANGLES,FINEANGLES*8,FINEANGLES,(VBLCOUNTER*3)/4);
        bufferofs+=screenofs;
        fizzlein = false;
        StartupClientControls();
    }

    bufferofs -= screenofs;

    UpdateClientControls ();

    if (HUD == true)
        DrawPlayerLocation();

    FlipPage();
    gamestate.frame++;

    player=tempptr;
}


//******************************************************************************
//
// FlipPage
//
//******************************************************************************

void FlipPage ( void )
{
    whereami=22;

    if ( ( SHAKETICS != 0xFFFF ) && ( !inmenu ) && ( !GamePaused ) &&
            ( !fizzlein ) )
    {
        ScreenShake ();
    }

    /* TODO some shake thing */

    /* just call the one in modexlib.c */
    XFlipPage();
}


//******************************************************************************
//
// TurnShakeOff
//
//******************************************************************************
void TurnShakeOff
(
    void
)

{
//   _disable();
    OUTP (CRTC_INDEX, CRTC_STARTHIGH );
    OUTP (CRTC_DATA, ( ( displayofs & 0x0000ffff ) >> 8 ) );
    OUTP (CRTC_INDEX, CRTC_STARTLOW);
    OUTP (CRTC_DATA, (displayofs&0x000000FF));
//   _enable();
    SHAKETICS = 0xFFFF;
}

//******************************************************************************
//
// DrawScaledScreen
// draw sreen after reentering fro restore game
//******************************************************************************

void DrawScaledScreen(int x, int y, int step, byte * src)
{
    int     xfrac;
    int     yfrac;
//    int     plane;
    int     i,j;
    byte    * p;
    byte    * buf;
    int     xsize;
    int     ysize;

    xsize=(iGLOBAL_SCREENWIDTH<<16)/step;
    if (xsize>iGLOBAL_SCREENWIDTH) xsize=iGLOBAL_SCREENWIDTH;
    ysize=(iGLOBAL_SCREENHEIGHT<<16)/step;
    if (ysize>iGLOBAL_SCREENHEIGHT) ysize=iGLOBAL_SCREENHEIGHT;

    {
        yfrac=0;
        for (j=y; j<y+ysize; j++)
        {
            p=src+(iGLOBAL_SCREENWIDTH*(yfrac>>16));
            buf=(byte *)bufferofs+ylookup[j]+x;
            xfrac=0;
            yfrac+=step;
            for (i=x; i<x+xsize; i++)
            {
                *buf=*(p+(xfrac>>16));
                buf++;
                xfrac+=step;
            }
        }
    }
}


//******************************************************************************
//
// DoLoadGameSequence
//
//******************************************************************************

void DoLoadGameSequence ( void )
{
    int x;
    int y;
    int dx;
    int dy;
    int s;
    int ds;
    int time;
    int i;
    byte * destscreen;
    pic_t *shape;//bna++





    fizzlein=false;
    x=(18+SaveGamePicX)<<16;
    y=(30+SaveGamePicY)<<16;
    time=VBLCOUNTER;
    s=0x2000000;
    dx=(-x)/time;
    dy=(-y)/time;
    ds=-((s-0x1000000)/time);

    destscreen=SafeMalloc(iGLOBAL_SCREENWIDTH*iGLOBAL_SCREENHEIGHT);
    SetupScreen(false);
    ThreeDRefresh();

    FlipPage();
    FlipPage();

    VL_CopyPlanarPageToMemory ( (byte *)bufferofs,  destscreen );

    CalcTics();
    
    DisableHudStretch();
    for (i=0; i<time; i+=tics)
    {
        CalcTics();
        DrawScaledScreen((x>>16),(y>>16),(s>>8),destscreen);
        FlipPage();
        x+=(dx*tics);
        if (x<0) x=0;
        y+=(dy*tics);
        if (y<0) y=0;
        s+=(ds*tics);
    }

    DrawScaledScreen(0,0,0x10000,destscreen);
    FlipPage();
    SafeFree(destscreen);
    CalcTics();
    CalcTics();
    //bna++ section
    shape =  ( pic_t * )W_CacheLumpName( "backtile", PU_CACHE, Cvt_pic_t, 1 );
    DrawTiledRegion( 0, 16, iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT - 32, 0, 16, shape );//bna++
    DrawPlayScreen(false);
    DisableScreenStretch();
    EnableHudStretch();
    SHAKETICS = 0xFFFF;
    //bna section end
}

//******************************************************************************
//
// StartupRotateBuffer
//
//******************************************************************************
byte * RotatedImage;
boolean RotateBufferStarted = false;
void StartupRotateBuffer ( int masked)
{
    int k;////zxcv
    int a,b;

//   int Xres = 320;//org
//   int Yres = 200;//org

    iG_masked = masked;

    if (RotateBufferStarted == true)
        return;

    RotateBufferStarted = true;

    //   RotatedImage=SafeMalloc(131072);org
    //RotatedImage=SafeMalloc(131072*8);
    
    
    //int amountToAlloc = ((iGLOBAL_SCREENWIDTH * iGLOBAL_SCREENHEIGHT)*2) + 3072; //this replaces 131072
    
    RotatedImage = SafeMalloc(131072);
    
/*
    if (iGLOBAL_SCREENWIDTH == 320) {
        RotatedImage=SafeMalloc(131072);
    } else if (iGLOBAL_SCREENWIDTH == 640) {
        RotatedImage=SafeMalloc(131072*4);
    } else if (iGLOBAL_SCREENWIDTH == 800) {
        RotatedImage=SafeMalloc(131072*8);
    }
    else if (iGLOBAL_SCREENWIDTH == 1024)
    {
        RotatedImage=SafeMalloc(131072*14);
    }
    else if (iGLOBAL_SCREENWIDTH == 1280)
    {
        RotatedImage=SafeMalloc(131072*20);
    }
    else if (iGLOBAL_SCREENWIDTH == 1400)
    {
        RotatedImage=SafeMalloc(131072*20);
    }
    else if (iGLOBAL_SCREENWIDTH == 1920)
    {
        RotatedImage=SafeMalloc(131072*50);
    }
*/
//SetupScreen(false);//used these 2 to test screen size
//VW_UpdateScreen ();
    if (masked==0) 
        memset(RotatedImage, 0, 131072);
    else
        memset(RotatedImage, 0xff, 131072);
/*
        if (iGLOBAL_SCREENWIDTH == 320) {
            memset(RotatedImage,0,131072);
        } else if (iGLOBAL_SCREENWIDTH == 640) {
            memset(RotatedImage,0,131072*4);
        } else if (iGLOBAL_SCREENWIDTH == 800) {
            //memset(RotatedImage,0,131072);//org
            memset(RotatedImage,0,131072*8);
        }else if (iGLOBAL_SCREENWIDTH == 1024) { 
            memset(RotatedImage,0,131072*14);
        }
        else if (iGLOBAL_SCREENWIDTH == 1280)
        {
            memset(RotatedImage, 0, 131072*20);
        }
        else if (iGLOBAL_SCREENWIDTH == 1400)
        {
            memset(RotatedImage, 0, 131072*20);
        }
        else if (iGLOBAL_SCREENWIDTH == 1920)
        {
            memset(RotatedImage, 0, 131072*50);
        }
    } else {
        if (iGLOBAL_SCREENWIDTH == 320) {
            memset(RotatedImage,0xff,131072);
        } else if (iGLOBAL_SCREENWIDTH == 640) {
            memset(RotatedImage,0xff,131072*4);
        } else if (iGLOBAL_SCREENWIDTH == 800) {
            memset(RotatedImage,0xff,131072*8);
        }
        else if (iGLOBAL_SCREENWIDTH == 1024) { 
            memset(RotatedImage,0xff,131072*14);
        }
        else if (iGLOBAL_SCREENWIDTH == 1280)
        {
            memset(RotatedImage, 0xff, 131072*20);
        }
        else if (iGLOBAL_SCREENWIDTH == 1400)
        {
            memset(RotatedImage, 0xff, 131072*20);
        }
        else if (iGLOBAL_SCREENWIDTH == 1920)
        {
            memset(RotatedImage, 0xff, 131072*50);
        }
*/
    
    //memset(RotatedImage,0xff,131072);//org
    //memset(RotatedImage,0xff,131072*8);

    if ((masked == false)&&(iGLOBAL_SCREENWIDTH >= 800)) {
        DisableScreenStretch();
        // SetTextMode (  );

        k=(28*512);//14336;
        //k=((0+28)<<10);//28672
        for (a=0; a<iGLOBAL_SCREENHEIGHT; a++) {
            for (b=0; b<iGLOBAL_SCREENWIDTH; b++) {
                //*(RotatedImage+99+((a+28)<<9)+b)   =   *((byte *)bufferofs+(a*linewidth)+b);
                // 99 is some offset value
                k = ((a+28)<<10);
                *(RotatedImage+(k)+b)   =   *((byte *)bufferofs+(a*linewidth)+b);
                //*(RotatedImage+b)   =   *((byte *)bufferofs+(a*linewidth)+b);
            }
            //k+=512*2;
        }
    } else if ((masked == false)&&(iGLOBAL_SCREENWIDTH == 640)) {
        DisableScreenStretch();
        k=(28*512);//14336;
        for (a=0; a<iGLOBAL_SCREENHEIGHT; a++) {
            for (b=0; b<iGLOBAL_SCREENWIDTH; b++) {
                k = ((a+28)<<10);
                *(RotatedImage+(k)+b)   =   *((byte *)bufferofs+(a*linewidth)+b);
            }
        }

    } else if ((masked == true)||(iGLOBAL_SCREENWIDTH == 320)) {
        for (a=0; a<200; a++) {
            for (b=0; b<320; b++)
                *(RotatedImage+99+((a+28)<<9)+b)=*((byte *)bufferofs+(a*linewidth)+b);
        }
    }

}
/* copier liner af 1024 bredde
a=0=14436 a=1=14848 a=2=15360 a=3=15872  -> 512 i difference
*(RotatedImage+(512)+0) = bufferofs+(0*800)+0);
*(RotatedImage+(512)+100) = bufferofs+(100*800)+0);
*/

//******************************************************************************
//
// ShutdownRotateBuffer
//
//******************************************************************************

void ShutdownRotateBuffer ( void )
{
    if (RotateBufferStarted == false)
        return;

    RotateBufferStarted = false;
    SafeFree(RotatedImage);
}

//******************************************************************************
//
// ScaleAndRotateBuffer
//
//******************************************************************************

void ScaleAndRotateBuffer (int startangle, int endangle, int startscale, int endscale, int time)
{
    int anglestep;
    int scalestep;
    int angle;
    int scale;
    int i;


//bna section
//   int Xh = 160;//org
//   int Yh = 100;//org

    int Xh = iGLOBAL_SCREENWIDTH/2;
    int Yh = iGLOBAL_SCREENHEIGHT/2;
//  Xh = 259;
//  Yh = 109;

    time = time;
////zxcv
    DisableScreenStretch();//bna++

    anglestep = (endangle-startangle)<<16/time;
    scalestep = (endscale-startscale)<<6/time;

    angle = startangle<<16;

    scale=(startscale<<6);

    CalcTics();
    CalcTics();
    //SDL_SetRelativeMouseMode(SDL_FALSE);
    
    for (i=0; i<time; i+=tics)
    {   //zxcv
        DrawRotatedScreen(Xh,Yh, (byte *)bufferofs,(angle>>16)&(FINEANGLES-1),scale>>6,0);
        
        FlipPage();
        scale+=(scalestep*tics);
        angle+=(anglestep*tics);
        CalcTics();
    }
    
    //FreeSDLSurfaceZoom();

    //SDL_SetRelativeMouseMode(SDL_TRUE);
    
    DrawRotatedScreen(Xh,Yh, (byte *)bufferofs,endangle&(FINEANGLES-1),endscale,0);
    FlipPage();
    DrawRotatedScreen(Xh,Yh, (byte *)bufferofs,endangle&(FINEANGLES-1),endscale,0);
    FlipPage();
    DrawRotatedScreen(Xh,Yh, (byte *)bufferofs,endangle&(FINEANGLES-1),endscale,0);
    CalcTics();
    CalcTics();
    //I_Delay (240);//bna++
    //bna++ section
    if ( playstate == ex_stillplaying )	  {//bna++
        pic_t *shape;
        shape =  ( pic_t * )W_CacheLumpName( "backtile", PU_CACHE, Cvt_pic_t, 1 );
        DrawTiledRegion( 0, 16, iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT - 32, 0, 16, shape );//bna++
        DisableScreenStretch();//dont strech when we go BACK TO GAME
        DrawPlayScreen(true);//repaint ammo and life stat
    }
}
//bna section end



//******************************************************************************
//
// RotateBuffer
//
//******************************************************************************

extern boolean skipRotate;

void RotateBuffer (int startangle, int endangle, int startscale, int endscale, int time)
{   
    int savetics;

    //save off fastcounter

    savetics=GetFastTics();
    
    StartupRotateBuffer (0);

    ScaleAndRotateBuffer (startangle, endangle, startscale, endscale, time);

    ShutdownRotateBuffer ();

    // restore fast counter
    SetFastTics(savetics);
}

const SDL_Renderer * GetRenderer(void);

void DoScreenRotateScale(int cx, int cy, SDL_Texture * tex, float angle, float scale);

void VL_FadeOutScaledScreen (int start, int end, int red, int green, int blue, int steps, float scale);


void RotateScreenScaleFloat(float startAngle, float endAngle, float startScale, float endScale, int time, boolean fadeOut, boolean drawPlayScreen)
{
    DisableScreenStretch();
    
    //STUB_FUNCTION;
    //printf("ROTATE SCREEN FLOAT FUNC: \n");
    //printf("startAngle: %f \n", startAngle);
    //printf("endAngle: %f \n", endAngle);
    //printf("startScale: %f \n", startScale);
    //printf("endScale: %f \n", endScale);
    //printf("time: %d \n", time);
    
    float angle = startAngle;
    
    float scalestep = (endScale - startScale)/(time);
    
    float scale = startScale;
    
    float anglestep = (endAngle - startAngle)/(time);
    
    //printf("anglestep: %f \n", anglestep);
    //printf("scalestep: %f \n", scalestep);
    //printf("startingScale: %f \n", scale);
    
    CalcTics();
    CalcTics();
    
    int i;
    
    SDL_Texture * newTex = SDL_CreateTextureFromSurface((SDL_Renderer *) GetRenderer(), sdl_surface);
    
    float factor;
    
    for (i=0; i<time; i+=tics)
    {
        factor = scale;
        
        //printf("factor: %f \n", factor);
        
        DoScreenRotateScale(iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT, newTex, angle, factor);
        
        scale+=(scalestep);
        //if (angle < endAngle)
        angle+=(anglestep);
        
        CalcTics();
        
        //printf("scaleDRAW: %f \n", scale);
    
    }
    
    //printf("factor: %f \n", factor);
    
    if(fadeOut == true)
        VL_FadeOutScaledScreen (0, 255, 0,0,0,VBLCOUNTER>>1, endScale);
    
    //DoScreenRotateScale(iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT, newTex, 0, 1.0);
    
    SDL_DestroyTexture(newTex);
    
    
    if(drawPlayScreen)
        DrawPlayScreen(true);//repaint ammo and life stat
    
}

//A note about option
//When option is set to
//    0: factor is set to equal scale value every time. This is good for starting from a small screen to eventually grow to a larger screen.
//    1: factor is set to equal 1 + scale value. This is good for zooming into the screen.
//    2: factor is set to equal 1 - scale value. This is good for zooming out from the screen.
/*
void RotateScreen(int startAngle, int endAngle, int startScale, int endScale, int time, int option, boolean fadeOut)
{
    DisableScreenStretch();
    
    //STUB_FUNCTION;
    
    printf("ROTATE SCREEN FUNC: \n");
    printf("startAngle: %d \n", startAngle);
    printf("endAngle: %d \n", endAngle);
    printf("startScale: %d \n", startScale);
    printf("endScale: %d \n", endScale);
    printf("time: %d \n", time);
    
    int angle = startAngle;
    
    float scalestep = (1.0 - ((float)startScale/(float)endScale))/((float)time);
    
    float scale = ((float)startScale/(float)endScale);
    
    if (option == 1)
    {
        scale = 1.0;
    }
    
    int anglestep = (endAngle - startAngle)/(time*5.49); //added *6 because it was rotating too effing fast
    
    //printf("anglestep: %d \n", anglestep);
    printf("scalestep: %f \n", scalestep);
    printf("startingScale: %f \n", scale);
    
    CalcTics();
    CalcTics();
    
    int i;
    
    int x;
    
    int y;
    
    SDL_Texture * newTex = SDL_CreateTextureFromSurface((SDL_Renderer *) GetRenderer(), sdl_surface);
    
    float factor;
    
    for (i=0; i<time; i+=tics)
    {
        if (option == 0)
            factor = scale;
        if (option == 1)
            factor = abs(1 + (scale*-1.0));
        if (option == 2)
            factor = 1 - scale;
        else
            factor = scale;
        
        //printf("factor: %f \n", factor);
        
        DoScreenRotateScale(iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT, newTex, angle, factor);
        
        scale+=(scalestep);
        if (angle < endAngle)
            angle+=(anglestep);
        
        CalcTics();
        
        //printf("scaleDRAW: %f \n", scale);
    
    }
    
    //printf("factor: %f \n", factor);
    
    if(fadeOut == true)
        VL_FadeOutScaledScreen (0, 255, 0,0,0,VBLCOUNTER>>1, abs(factor));
    
    //DoScreenRotateScale(iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT, newTex, 0, 1.0);
    
    SDL_DestroyTexture(newTex);
    
    DrawPlayScreen(true);//repaint ammo and life stat

}
*/


//******************************************************************************
//
// DrawRotatedScreen
//
//******************************************************************************

void DrawRotatedScreen(int cx, int cy, byte *destscreen, int angle, int scale, int masked)
{   //ZXCV
    int     c, s;
    int     xst, xct;
    int     y;

    byte    * screen;
    //int Xres = 320;//old value
    //int Yres = 200;//old val

    int Xr = iGLOBAL_SCREENWIDTH;//640;
    int Yr = (iGLOBAL_SCREENHEIGHT);//400; //bna aaaa fix

//	   SetTextMode (  );
    c = FixedMulShift(scale,costable[angle],11);
    s = FixedMulShift(scale,sintable[angle],11);

//   c = c/2; //these values are to rotate degres or?
//   s = s/2;
//   xst & xct= start center values ;
    if ((iGLOBAL_SCREENWIDTH == 320 )||(masked == true)) {
        xst = (((-cx)*s)+(128<<16))-(cy*c);
        xct = (((-cx)*c)+(256<<16)+(1<<18)-(1<<16))+(cy*s);
    }
    else if ((iGLOBAL_SCREENWIDTH == 640 )&&(masked == false)) {
        xst = (((-cx)*s)+((268)<<16))-(cy*c);
        xct = (((-cx)*c)+((317)<<16)+(1<<18)-(1<<16))+(cy*s);
    }//y=268;x=317
    else if ((iGLOBAL_SCREENWIDTH == 800 )&&(masked == false)) {
        xst = (((-cx)*s)+((328)<<16))-(cy*c);
        xct = (((-cx)*c)+((397)<<16)+(1<<18)-(1<<16))+(cy*s);
    }//328 397
    else if ((iGLOBAL_SCREENWIDTH == 1024 )&&(masked == false)) {
	xst = (((-cx)*s)+((410)<<16))-(cy*c);// 1024/768=1.3333
	xct = (((-cx)*c)+((500)<<16)+(1<<18)-(1<<16))+(cy*s);
    }//388 397
    else if ((iGLOBAL_SCREENWIDTH >= 1152) && (masked == false))
    {
        xst = (((-cx)*s)+((432)<<16))-(cy*c);
	xct = (((-cx)*c)+((576)<<16)+(1<<18)-(1<<16))+(cy*s);
    }

    mr_xstep=s;
    mr_ystep=c;


    if ((iGLOBAL_SCREENWIDTH >= 800)&&(masked==0)) {
        screen=destscreen+iGLOBAL_SCREENWIDTH;//bna aaaa fix
    } else {
        screen=destscreen;
    }

    if (masked==0)
    {
        // paint hole 800x600 screen
        {
            mr_yfrac=xct;
            mr_xfrac=xst;
            for (y=0; y<Yr; y++,mr_xfrac+=c,mr_yfrac-=s)
                DrawRotRow(Xr,screen+ylookup[y],RotatedImage);
        }
    }
    else
    {
        {
            mr_yfrac=xct;
            mr_xfrac=xst;
            for (y=0; y<Yr; y++,mr_xfrac+=c,mr_yfrac-=s)
                DrawMaskedRotRow(Xr,screen+ylookup[y],RotatedImage);
        }
    }

}


//******************************************************************************
//
// DrawScaledPost
//
//******************************************************************************

void DrawScaledPost ( int height, byte * src, int offset, int x)
{
    patch_t *p;

    p=(patch_t *)src;
    dc_invscale=(height<<16)/p->origsize;
    dc_iscale=(p->origsize<<16)/height;
    dc_texturemid=(((p->origsize>>1)+p->topoffset)<<SFRACBITS)+(SFRACUNIT>>1);
    sprtopoffset=centeryfrac - FixedMul(dc_texturemid,dc_invscale);
    shadingtable=colormap+(1<<12);
    ScaleMaskedPost(((p->collumnofs[offset])+src), (byte *)bufferofs+x);
}



void ApogeeTitle (void)
{
    byte pal[768];
    int   angle;
    int   scale;
    int   x,y;
    int   danglex;
    int   anglex;
    int   dy,dangle,dscale;
    int   time;

    CalcTics();
    CalcTics();
    IN_ClearKeysDown();
    viewwidth=320;
    viewheight=200;
    memcpy(&pal[0],W_CacheLumpName("ap_pal",PU_CACHE, CvtNull, 1),768);
    shadingtable=colormap+(1<<12);
    VL_NormalizePalette(&pal[0]);
    SwitchPalette(&pal[0],35);
//   DrawWorld();
//   RotateBuffer(0,FINEANGLES*6,FINEANGLES*48,FINEANGLES,(VBLCOUNTER*2));
//   DoLaserShoot("apogee");
//   DoZIntro();

    VL_ClearBuffer (bufferofs, 255);
    DrawNormalSprite (0, 0, W_GetNumForName("ap_titl"));

    StartupRotateBuffer (1);

    //save off fastcounter

#define APOGEEXANGLE 913
#define APOGEEXMAG   180
#define APOGEESTARTY 0
#define APOGEEENDY   100

#define APOGEESCALESTART (FINEANGLES<<4)
#define APOGEESCALEEND (FINEANGLES)
#define APOGEESONGTIME (124-1)

    time = APOGEESONGTIME;

    anglex=0;
    danglex=(APOGEEXANGLE<<16)/time;

    y=APOGEESTARTY<<16;
    dy=((APOGEEENDY-APOGEESTARTY)<<16)/time;

    dscale=((APOGEESCALEEND-APOGEESCALESTART)<<16)/time;
    scale=APOGEESCALESTART<<16;

    angle=0;
    dangle=(FINEANGLES<<17)/time;

    MU_StartSong(song_apogee);

    CalcTics();

    while (time>=0)
    {
        VL_DrawPostPic (W_GetNumForName("ap_wrld"));
        IN_PumpEvents();

        x=100+FixedMul(APOGEEXMAG,sintable[anglex>>16]);

        DrawRotatedScreen(x,y>>16,(byte *)bufferofs,(angle>>16)&(FINEANGLES-1),scale>>16,1);
        FlipPage();
        CalcTics();
        angle+=dangle*tics;
        scale+=dscale*tics;
        y+=dy*tics;
        anglex+=danglex*tics;
        time-=tics;
        if ((LastScan) || IN_GetMouseButtons())
            goto apogeeexit;
    }
    CalcTics();
    CalcTics();
    VL_DrawPostPic (W_GetNumForName("ap_wrld"));
    DrawRotatedScreen(x,y>>16,(byte *)bufferofs,0,APOGEESCALEEND,1);
    FlipPage();

    while (MU_SongPlaying())
    {
        IN_PumpEvents();
        if ((LastScan) || IN_GetMouseButtons())
            goto apogeeexit;
    }

//  I_Delay(65); //bna added
apogeeexit:

    VL_ClearBuffer (bufferofs, 0); //bna added
    MenuFadeOut(); //bna added
    VH_UpdateScreen (); //bna added
    ShutdownRotateBuffer ();

}

#if (SHAREWARE==0)

void DopefishTitle (void)
{
    int shapenum;
    int height;

    shapenum=W_GetNumForName("scthead1");
    CalcTics();
    CalcTics();
    IN_ClearKeysDown();
    MU_StartSong( song_secretmenu);
    viewwidth=320;
    viewheight=200;
    SwitchPalette(origpal,35);
    oldtime=GetTicCount();
    FlipPage();
    for (height=1; height<200; height+=(tics<<2))
    {
        DrawPositionedScaledSprite (160, 100, shapenum, height, 0);
        FlipPage();
        CalcTics();
        if ((LastScan) || IN_GetMouseButtons())
            break;
    }
    SD_Play ( SD_DOPEFISHSND );
    oldtime=GetTicCount();
    for (height=0; height<FINEANGLES<<1; height+=(tics<<5))
    {
        DrawPositionedScaledSprite (160+FixedMul(60,costable[height&(FINEANGLES-1)]), 100+FixedMul(60,sintable[height&(FINEANGLES-1)]), shapenum, 200, 0);
        FlipPage();
        VL_CopyPlanarPage ( (byte *) displayofs, (byte *) bufferofs );
        CalcTics();
        if ((LastScan) || IN_GetMouseButtons())
            break;
    }
    SD_Play ( SD_DOPEFISHSND );
    FlipPage();
}

#endif

void RotationFunSDL(void)
{
    int   angle;
    float   scale;
    int   x,y;
    word  buttons;

    //save off fastcounter


    angle=0;
    //scale=FINEANGLES;
    scale=1.0;

    //StartupRotateBuffer (0);

    CalcTics();
    CalcTics();
    
    SDL_Texture * currScreen = SDL_CreateTextureFromSurface((SDL_Renderer *) GetRenderer(), sdl_surface);
    
    while (!Keyboard[sc_Escape])
    {
        IN_UpdateKeyboard ();
        //printf("PRE SCALE \n");
        
        DoScreenRotateScale(iGLOBAL_SCREENWIDTH, iGLOBAL_SCREENHEIGHT, 
                            currScreen, angle, scale);
        
        //printf("POST SCALE \n");
        //DrawRotatedScreen(iGLOBAL_SCREENWIDTH/2,iGLOBAL_SCREENHEIGHT/2,(byte *)bufferofs,angle,scale,0);
        //FlipPage();
        CalcTics();
        INL_GetMouseDelta(&x, &y);
        buttons=IN_GetMouseButtons ();
        angle=(angle-x)&(FINEANGLES-1);
        if (buttons & (1 << 0))
        {
            if (scale>0)
            {
                scale-=0.01;
                //printf("new width:  %f \n", (float)iGLOBAL_SCREENWIDTH * scale);
                //printf("new height: %f \n", (float)iGLOBAL_SCREENHEIGHT * scale);
                //printf("scale: %f \n", scale);
                
            }
        }
        else if (buttons & (1 << 1))
        {
            scale+=0.01;
            //printf("new width:  %f \n", (float)iGLOBAL_SCREENWIDTH * scale);
            //printf("new height: %f \n", (float)iGLOBAL_SCREENHEIGHT * scale);
            //printf("scale: %f \n", scale);
        }
    }
    SDL_DestroyTexture(currScreen);
    
    CalcTics();
    CalcTics();
    Keyboard[sc_Escape]=0;

    //ShutdownRotateBuffer ();
}


//******************************************************************************
//
// RotationFun
//
//******************************************************************************

void RotationFun ( void )
{
    int   angle;
    int   scale;
    int   x,y;
    word  buttons;

    //save off fastcounter


    angle=0;
    scale=FINEANGLES;

    StartupRotateBuffer (0);

    CalcTics();
    CalcTics();
    while (!Keyboard[sc_Escape])
    {
        IN_UpdateKeyboard ();
        
        DrawRotatedScreen(iGLOBAL_SCREENWIDTH/2,iGLOBAL_SCREENHEIGHT/2,(byte *)bufferofs,angle,scale,0);
        FlipPage();
        CalcTics();
        INL_GetMouseDelta(&x, &y);
        buttons=IN_GetMouseButtons ();
        angle=(angle-x)&(FINEANGLES-1);
        if (buttons & (1 << 0))
        {
            if (scale>0)
                scale-=30;
        }
        else if (buttons & (1 << 1))
        {
            scale+=30;
        }
    }
    CalcTics();
    CalcTics();
    Keyboard[sc_Escape]=0;

    ShutdownRotateBuffer ();
}

boolean ScreenSaverStarted=false;
screensaver_t * ScreenSaver;
#define PAUSETIME  (70)

//******************************************************************************
//
// SetupScreenSaverPhase
//
//******************************************************************************
void SetupScreenSaverPhase ( void )
{
    if (ScreenSaverStarted==false)
        return;

    if (ScreenSaver->phase==0)
    {
        ScreenSaver->x=160;
        ScreenSaver->y=100;
        ScreenSaver->angle=0;
        ScreenSaver->scale=FINEANGLES;
        ScreenSaver->dangle=FINEANGLES/VBLCOUNTER;
        ScreenSaver->dx=0;
        ScreenSaver->dy=0;
        ScreenSaver->dscale=((FINEANGLES<<2)-(FINEANGLES))/VBLCOUNTER;
        ScreenSaver->time=VBLCOUNTER;
    }
    else if (ScreenSaver->phase==1)
    {
        ScreenSaver->x=160;
        ScreenSaver->y=100;
        ScreenSaver->angle=0;
        ScreenSaver->scale=FINEANGLES<<2;
        ScreenSaver->dangle=FINEANGLES/VBLCOUNTER;
        ScreenSaver->dx=RandomNumber("StartupScreen",0)>>5;
        ScreenSaver->dy=RandomNumber("StartupScreen",0)>>5;
        ScreenSaver->dscale=0;
        ScreenSaver->time=-1;
    }
}

//******************************************************************************
//
// StartupScreenSaver
//
//******************************************************************************
void StartupScreenSaver ( void )
{
    if (ScreenSaverStarted==true)
        return;

    ScreenSaverStarted=true;

    StartupRotateBuffer (0);

    ScreenSaver=(screensaver_t *)SafeMalloc(sizeof(screensaver_t));
    ScreenSaver->phase=0;
    ScreenSaver->pausetime=PAUSETIME;
    if (iGLOBAL_SCREENWIDTH == 320) {
        ScreenSaver->pausex=120;
        ScreenSaver->pausey=84;
    } else if (iGLOBAL_SCREENWIDTH == 640) {
        ScreenSaver->pausex=240;
        ScreenSaver->pausey=201;
    } else if (iGLOBAL_SCREENWIDTH == 800) {
        ScreenSaver->pausex=300;
        ScreenSaver->pausey=252;
    }
    ScreenSaver->pausex=120;
    ScreenSaver->pausey=84;
    SetupScreenSaverPhase();
}

//******************************************************************************
//
// ShutdownScreenSaver
//
//******************************************************************************
void ShutdownScreenSaver ( void )
{
    if (ScreenSaverStarted==false)
        return;

    ScreenSaverStarted=false;

    ShutdownRotateBuffer ();
    SafeFree(ScreenSaver);
    //bna section
    StartupClientControls();

}

//******************************************************************************
//
// UpdateScreenSaver
//
//******************************************************************************

#define SPINSIZE  40
#define MAXSPEED  8
void UpdateScreenSaver ( void )
{
    //EnableScreenStretch();
    if (ScreenSaver->time!=-1)
    {
        ScreenSaver->time-=tics;
        if (ScreenSaver->time<0)
        {
            ScreenSaver->phase++;
            SetupScreenSaverPhase();
        }
    }
    ScreenSaver->x+=ScreenSaver->dx*tics;
    ScreenSaver->y+=ScreenSaver->dy*tics;
    ScreenSaver->angle=(ScreenSaver->angle+(ScreenSaver->dangle*tics))&(FINEANGLES-1);
    ScreenSaver->scale+=ScreenSaver->dscale*tics;
    if (ScreenSaver->x<SPINSIZE)
    {
        ScreenSaver->x=SPINSIZE;
        ScreenSaver->dx=abs(ScreenSaver->dx);
        ScreenSaver->dy+=(RandomNumber("Rotate",0)>>6)-2;
    }
    else if (ScreenSaver->x>iGLOBAL_SCREENWIDTH-SPINSIZE)
    {
        ScreenSaver->x=iGLOBAL_SCREENWIDTH-SPINSIZE;
        ScreenSaver->dx=-(abs(ScreenSaver->dx));
        ScreenSaver->dy+=(RandomNumber("Rotate",0)>>6)-2;
    }
    if (ScreenSaver->y<SPINSIZE)
    {
        ScreenSaver->y=SPINSIZE;
        ScreenSaver->dy=abs(ScreenSaver->dy);
        ScreenSaver->dx+=(RandomNumber("Rotate",0)>>6)-2;
    }
    else if (ScreenSaver->y>iGLOBAL_SCREENHEIGHT-SPINSIZE)
    {
        ScreenSaver->y=iGLOBAL_SCREENHEIGHT-SPINSIZE;
        ScreenSaver->dy=-(abs(ScreenSaver->dy));
        ScreenSaver->dx+=(RandomNumber("Rotate",0)>>6)-2;
    }

    if (abs(ScreenSaver->dx)>MAXSPEED)
        ScreenSaver->dx=SGN(ScreenSaver->dx)*MAXSPEED;

    if (abs(ScreenSaver->dy)>MAXSPEED)
        ScreenSaver->dy=SGN(ScreenSaver->dy)*MAXSPEED;

    DrawRotatedScreen(ScreenSaver->x,ScreenSaver->y, (byte *)bufferofs,ScreenSaver->angle,ScreenSaver->scale,0);

    ScreenSaver->pausetime-=tics;
    if (ScreenSaver->pausetime<=0)
    {
        ScreenSaver->pausetime=PAUSETIME;
        if (iGLOBAL_SCREENWIDTH == 320) {
            ScreenSaver->pausex=RandomNumber ("pausex",0)%240;
            ScreenSaver->pausey=RandomNumber ("pausey",0)%168;
        } else if (iGLOBAL_SCREENWIDTH == 640) {
            ScreenSaver->pausex=RandomNumber ("pausex",0)%480;
            ScreenSaver->pausey=RandomNumber ("pausey",0)%403;
        } else if (iGLOBAL_SCREENWIDTH == 800) {
            ScreenSaver->pausex=RandomNumber ("pausex",0)%600;
            ScreenSaver->pausey=RandomNumber ("pausey",0)%504;
        }
    }
    DrawPauseXY (ScreenSaver->pausex, ScreenSaver->pausey);

    FlipPage();
}



// Old Stuff

/*
   int y1;
   int y2;

   if (post->alttile!=0)
      {
      ht=nominalheight;
      src2=W_CacheLumpNum(1+post->alttile,PU_CACHE);
//      src2+=8;
      }
   else
      {
      ht=maxheight;
      src2=src;
      }
   hp_srcstep=(64<<(16+HEIGHTFRACTION))/post->wallheight;
   y1 = (((centery<<HFRACTION)-(post->wallheight*pheight)+(1<<(HFRACTION-1))));
   y2 = (((post->wallheight*ht)+y1)>>HFRACTION);

   if ((y1>>HFRACTION)>=viewheight)
      {
      post->ceilingclip=viewheight-1;
      post->floorclip=viewheight-1;
      return;
      }
   else if (y1<0)
      {
      hp_startfrac=FixedMulShift(-y1,hp_srcstep,HFRACTION);
      y1=0;
      post->ceilingclip=0;
      }
   else
      {
      hp_startfrac=FixedMulShift(255-(y1&0xff),hp_srcstep,HFRACTION);
      y1>>=HFRACTION;
      post->ceilingclip=y1;
      }
   if (y2<0)
      {
      post->floorclip=0;
      post->ceilingclip=0;
      }
   else if (y2>viewheight)
      {
      DrawHeightPost(viewheight-y1, src2+((post->texture>>4)&0xfc0), buf+ylookup[y1]);
      post->floorclip=viewheight-1;
      }
   else
      {
      DrawHeightPost(y2-y1, src2+((post->texture>>4)&0xfc0), buf+ylookup[y1]);
      post->floorclip=y2-1;
      }

   if (ht==maxheight)
      return;

   y1 = (((centery<<HFRACTION)-(post->wallheight*(pheight-ht))+(1<<(HFRACTION-1))));
   y2 = (((post->wallheight<<6)+y1)>>HFRACTION);

   if ((y1>>HFRACTION)>=viewheight)
      return;
   else if (y1<0)
      {
      hp_startfrac=FixedMulShift(-y1,hp_srcstep,HFRACTION);
      y1=0;
      }
   else
      {
      hp_startfrac=FixedMulShift(255-(y1&0xff),hp_srcstep,HFRACTION);
      y1>>=HFRACTION;
      }
   if (y2<0)
      return;
   else if (y2>viewheight)
      {
      DrawHeightPost(viewheight-y1, src+((post->texture>>4)&0xfc0), buf+ylookup[y1]);
      post->floorclip=viewheight-1;
      }
   else
      {
      DrawHeightPost(y2-y1, src+((post->texture>>4)&0xfc0), buf+ylookup[y1]);
      post->floorclip=y2-1;
      }
}
*/






//******************************************************************************
//
// DrawBackground
//
//******************************************************************************

void DrawBackground ( byte * bkgnd )
{
//   int plane;
    int size;

    size=linewidth*200;
    {
        memcpy((byte *)bufferofs,bkgnd,size);
        bkgnd+=size;
    }
}


//******************************************************************************
//
// PrepareBackground
//
//******************************************************************************

void PrepareBackground ( byte * bkgnd )
{
//   int plane;
    int size;

    size=linewidth*200;
    {
        memcpy(bkgnd,(byte *)bufferofs,size);
        bkgnd+=size;
    }
}

//******************************************************************************
//
// WarpString
//
//******************************************************************************

void WarpString (
    int x, int y, int endx, int endy,
    int time, byte * back, char * str
)
{
    int dx;
    int dy;
    int cx;
    int cy;

    LastScan = 0;


    dx=((endx-x)<<16)/time;
    dy=((endy-y)<<16)/time;
    cx=x<<16;
    cy=y<<16;

    CalcTics();

    while (time>0)
    {

        DrawBackground ( back );
        US_ClippedPrint (cx>>16, cy>>16, str);
        FlipPage();

        CalcTics();
        cx+=dx*tics;
        cy+=dy*tics;
        time-=tics;
        if (LastScan != 0)
            break;
    }

    // DrawBackground ( back );
    // US_ClippedPrint (endx, endy, str);
    // FlipPage();

}


#if (SHAREWARE==1)
//******************************************************************************
//
// DoEndCinematic
//
//******************************************************************************

//******************************************************************************
//
// WarpSprite
//
//******************************************************************************

void WarpSprite (
    int x, int y, int endx, int endy,
    int time, byte * back, int shape
)
{
    int dx;
    int dy;
    int cx;
    int cy;
    int starttime;

    LastScan = 0;

    dx=((endx-x)<<16)/time;
    dy=((endy-y)<<16)/time;
    cx=x<<16;
    cy=y<<16;
    starttime=time;

    CalcTics();

    while (time>0)
    {
        DrawBackground ( back );
        DrawUnScaledSprite (cx>>16, cy>>16, shape, 16);
        FlipPage();
        CalcTics();
        cx+=dx*tics;
        cy+=dy*tics;
        time-=tics;
        if (LastScan != 0)
            break;
    }
}


char *EndCinematicPicNames[5] =
{
    "lwgshoo2",
    "hg2shoo2",
    "ankshoo1",
    "ligrise4",
    "tritoss5",

};

#define NUMENDMESSAGES 24


char *EndCinematicText[NUMENDMESSAGES] =
{
    "You've won the battle, Cassatt.\n"
    "But when the Oscuridos return,\n"
    "will you be ready as they wage\n"
    "their Dark War?",

    "Armed with only a pistol and 30\n"
    "bucks, you must stop the minions of\n"
    "El Oscuro before they kill millions\n"
    "of innocent people.",

    "But for now, hey, enjoy the medal\n"
    "you received and take a vacation.\n"
    "You've earned it. Maybe on \n"
    "San Nicolas Island . . .",

    "Thanks for playing. If you liked\n"
    "\"The HUNT Begins\", check Ordering\n"
    "Info for information about \n"
    "continuing your adventure.",

    "Okay, you can stop reading now.",

    "Press a key. That's all there is.\n"
    "Thanks.",

    "Are you lazy, or illiterate?\n"
    "PRESS A KEY.",

    "Look, this is pointless. You\n"
    "are done. Push off.",

    "Okay, show's over.  Nothing\n"
    "more to see here.",

    "Wow, you must like this fine\n"
    "background screen.",

    "For waiting this long, you get . . .\n"
    "nothing!  Go away!",

    "I mean, I like you as a friend,\n"
    "but . . .",

    "\"Bob\"",

    "All right, um . . . you found the\n"
    "secret message! Congratulations!",

    "Didn't work, huh?  Okay, how about\n"
    "this . . .",

    "THE END",

    "Dang. Thought I had you there.",

    "Stop watching.",

    "You know that if you registered,\n"
    "there would be a lot more cool\n"
    "stuff happening right now.",

    "Episode IV: A New Hope\n",

    "Just think of all the new secret\n"
    "messages you could find hidden\n"
    "in the registered version!",

    "Someone right now is probably\n"
    "enjoying the really exciting\n"
    "ending of the registered version.",

    "ROTT was filmed before\n"
    "a live audience.",

    "No animals were harmed during the\n"
    "creation of this video game, although\n"
    "one dog did get its butt spanked\n"
    "when it peed on the carpet.\n",


};
char NextGameString1[] = "The Developers of Incredible Power";
char NextGameString2[] = "shall return";

void DoEndCinematic ( void )
{
    int trilogo;
    int group;
    int world;
    int width;
    int height;
    int x,y;
    int shape;
    int time1,time2;
    byte * tmp;
    byte * sky;
    byte * bkgnd;
    int i;

    byte pal[768];
    EnableScreenStretch();

    viewwidth = 320;//MAXSCREENWIDTH;
    viewheight = 200;//MAXSCREENHEIGHT;

    MU_StartSong(song_youwin);

    bkgnd=SafeMalloc(800*linewidth);

    trilogo=W_GetNumForName("trilogo");
    world=W_GetNumForName("ap_wrld");
    group=W_GetNumForName("mmbk");
    VL_DrawPostPic (trilogo);
    PrepareBackground ( bkgnd );

    WarpSprite (160, -100, 160, 100, (VBLCOUNTER*3), bkgnd, W_GetNumForName("youwin"));
    if (LastScan !=0)
        goto fadelogo;

    I_Delay(30);
fadelogo:
    MenuFadeOut();
    ClearGraphicsScreen();
    memcpy(&pal[0],W_CacheLumpName("ap_pal",PU_CACHE,CvtNull,1),768);
    VL_NormalizePalette(&pal[0]);
    SwitchPalette(&pal[0],35);

    VL_DrawPostPic (world);
    PrepareBackground ( bkgnd );

    WarpSprite (160, 250, 160, 100, (VBLCOUNTER*3), bkgnd, W_GetNumForName("wrldsafe"));
    if (LastScan !=0)
        goto fadeworld;

    I_Delay(10);
    if (LastScan !=0)
        goto fadeworld;

    WarpSprite (160, 100, 160, -50, (VBLCOUNTER*3), bkgnd, W_GetNumForName("wrldsafe"));
    if (LastScan !=0)
        goto fadeworld;

    I_Delay(20);

fadeworld:
    MenuFadeOut();
    ClearGraphicsScreen();
    MenuFadeIn();


    sky=W_CacheLumpNum(W_GetNumForName("SKYSTART")+2,PU_CACHE,CvtNull,1);
    tmp=sky;
    for (x=0; x<256; x++)
    {
        for (y=0; y<200; y++)
        {
            *((byte *)bufferofs+ylookup[y]+x)=*tmp++;
        }
    }
    tmp=sky;
    for (x=256; x<320; x++)
    {
        for (y=0; y<200; y++)
        {
            *((byte *)bufferofs+ylookup[y]+x)=*tmp++;
        }
    }

    for(i=0; i<5; i++)
    {
        int tx,ty;

        tx = 32 + (i << 6);
        ty = 100;
        shape = W_GetNumForName(EndCinematicPicNames[i]);
        DrawUnScaledSprite (tx, ty, shape,16);
    }

    PrepareBackground ( bkgnd );

    //CurrentFont = (font_t *)W_CacheLumpName ("newfnt1", PU_CACHE);
    CurrentFont = smallfont;
    LastScan = 0;

    for(i=0; i<NUMENDMESSAGES; i++)
    {
        if (i>3)
            I_Delay(50);

        US_MeasureStr (&width, &height, "%s", &(EndCinematicText[i][0]));
        if (LastScan !=0)
            break;

        x=(320-width)>>1;
        y=(200-height)>>1;
        time1 = (300 - y)*(VBLCOUNTER*4)/300;
        time2 = VBLCOUNTER*4-time1;

        WarpString (x, 250, x, y-50,time1, bkgnd, EndCinematicText[i]);
        if (LastScan !=0)
            break;
        I_Delay(40);
        if (LastScan !=0)
            break;

        if (i<=3)
            I_Delay(40);
        if (LastScan !=0)
            break;

        WarpString (x, y-50, x, -50, time2, bkgnd, EndCinematicText[i]);
        if (LastScan !=0)
            break;


    }

    if (LastScan!=0)
        goto finalfade;

    sky=W_CacheLumpNum(W_GetNumForName("SKYSTART")+2,PU_CACHE,CvtNull,1);
    tmp=sky;
    for (x=0; x<256; x++)
    {
        for (y=0; y<200; y++)
        {
            *((byte *)bufferofs+ylookup[y]+x)=*tmp++;
        }
    }
    tmp=sky;
    for (x=256; x<320; x++)
    {
        for (y=0; y<200; y++)
        {
            *((byte *)bufferofs+ylookup[y]+x)=*tmp++;
        }
    }

    for(i=0; i<5; i++)
    {
        int tx,ty;

        tx = 32 + (i << 6);
        ty = 100;
        shape = W_GetNumForName(EndCinematicPicNames[i]);
        DrawUnScaledSprite (tx, ty, shape,16);
    }


    shape = W_GetNumForName("robogrd3");
    PrepareBackground ( bkgnd );
    WarpSprite (420,100,300,100,VBLCOUNTER*3,bkgnd,shape);
    if (LastScan !=0)
        goto finalfade;

    PrepareBackground ( bkgnd );
    WarpString (200,80,200,80,VBLCOUNTER*3,bkgnd, "Am I late?");
    if (LastScan !=0)
        goto finalfade;


    I_Delay(20);
finalfade:

    MenuFadeOut();
    VL_ClearVideo (0);
    I_Delay(10);

    if (LastScan == 0)
    {
        US_MeasureStr (&width, &height, "%s", NextGameString1);
        x=(320-width)>>1;
        y=(200-height)>>1;
        US_ClippedPrint (x,y-6, NextGameString1);
        US_MeasureStr (&width, &height, "%s", NextGameString2);
        x=(320-width)>>1;
        y=(200-height)>>1;
        US_ClippedPrint (x,y+6, NextGameString2);
        FlipPage();
        VL_FadeIn(0,255,origpal,150);
        I_Delay(50);
        VL_FadeOut(0,255,0,0,0,150);
        VL_ClearVideo (0);
        I_Delay(10);
    }

    SafeFree(bkgnd);
}
#else

// REGISTERED VERSION ======================================================

static char    burnCastle1Msg []=
    "The monastery burns.\n"
    "\n"
    "El Oscuro is dead.\n"
    "\n"
    "The world is safe.\n";

// If all Snake Eggs not destroyed on final level:


static char    notDoneMsg[] =
    "Unfortunately not all\n"
    "of El Oscuro's larvae\n"
    "were destroyed.\n"
    "\n"
    "Thirty years later,\n"
    "a descendant of\n"
    "El Oscuro wiped out\n"
    "the entire world,\n"
    "but nice job anyway.\n";

static char    tryAgainMsg[] =
    "Try Again.\n"
    "\n"
    "The world will not be\n"
    "safe until all of El\n"
    "Oscuro's larvae are\n"
    "destroyed. Find them.\n";

// If all snake eggs destroyed:
static char    doneMsg[] =
    "You have destroyed\n"
    "El Oscuro and all his\n"
    "descendants.  Well done!\n";

// On Triad background, in bigger font.
static char    youWin1Msg[] =
    "So, HUNT Members, how\n"
    "do you think the\n"
    "mission went?\n";

// Place menu pix of characters here (maybe modem frame too?)
static char    youWin2Msg[] =
    "Barrett: Well, I think\n"
    "I got shin splints from\n"
    "all those jump pads.\n"
    "But hey, action-wise,\n"
    "I've been in tougher\n"
    "bar fights, for crying\n"
    "out loud.\n";

static char    youWin3Msg[] =
    "Cassatt: Apart from\n"
    "the other HUNT members\n"
    "saying I look like\n"
    "Richard Mulligan, it\n"
    "was quite a success.\n"
    "And some of the\n"
    "monastery's ironwork\n"
    "was very nice.\n";

static char    youWin4Msg[] =
    "Ni: it was quite easy,\n"
    "actually.  I just\n"
    "pictured the enemy\n"
    "having the face of\n"
    "my ex-husband, and\n"
    "man, I was a force\n"
    "of Nature.\n";

static char    youWin5Msg[] =
    "Wendt: I was kind of\n"
    "disappointed. I think\n"
    "I used the missile\n"
    "weapons way too much.\n"
    "Next time, bullets\n"
    "only.  Nothing sweeter\n"
    "than a head shot from\n"
    "a hundred feet.\n";

static char    youWin6Msg[] =
    "Freeley: I'm still\n"
    "trying to adjust in\n"
    "the aftermath.  It's\n"
    "kinda tough.  I mean,\n"
    "I save the damn world,\n"
    "and all people ask\n"
    "about is my name.\n"
    "Sheesh.\n";

// On caching screen

static char     youWin7Msg[] =
    "The HUNT is victorious!\n"
    "\n"
    "         THE END\n";


static char     youWin8Msg[] =
    "Now go and celebrate!\n"
    "\n"
    "      THE REAL END";

#define NUMEXPLOSIONTYPES 4

typedef struct {
    char  name[11];
    byte  numframes;
} ExplosionInfoType;

ExplosionInfoType ExplosionInfo[NUMEXPLOSIONTYPES]=
{
    {"EXPLOS1\0",20},
    {"EXP1\0",20},
    {"GREXP1\0",25},
    {"PART1\0",12},
};


typedef struct {
    byte  which;
    byte  frame;
    byte  x;
    byte  y;
} ExplosionType;

#define MAXTRANSMITTEREXPLOSIONS 30

static ExplosionType Explosions[MAXTRANSMITTEREXPLOSIONS];

void ResetTransmitterExplosion ( ExplosionType * Explosion )
{
    Explosion->which=RandomNumber("Explosion",0)%NUMEXPLOSIONTYPES;
    Explosion->frame=0;
    Explosion->x=(RandomNumber("Explosion",2)>>1)+(160-64);
    Explosion->y=(RandomNumber("Explosion",3)>>1);
}

void CacheTransmitterExplosions ( void )
{
    int i,j,num;

    for (i=0; i<NUMEXPLOSIONTYPES; i++)
    {
        num=W_GetNumForName(ExplosionInfo[i].name);
        for (j=0; j<ExplosionInfo[i].numframes; j++)
        {
            W_CacheLumpNum(num+j, PU_CACHE, Cvt_patch_t, 1);
        }
    }
}

void SetupTransmitterExplosions ( void )
{
    int i;

    for (i=0; i<MAXTRANSMITTEREXPLOSIONS; i++)
    {
        ResetTransmitterExplosion(&Explosions[i]);
    }
}
void UpdateTransmitterExplosions ( void )
{
    int i;
    for (i=0; i<MAXTRANSMITTEREXPLOSIONS; i++)
    {
        Explosions[i].frame+=tics;
        if (Explosions[i].frame>=(ExplosionInfo[Explosions[i].which].numframes<<1))
        {
            ResetTransmitterExplosion(&Explosions[i]);
            SD_Play(SD_EXPLODEFLOORSND+(RandomNumber("Explosion",4)>>7));
        }
    }
}

void DrawTransmitterExplosions ( void )
{
    int i;
    for (i=0; i<MAXTRANSMITTEREXPLOSIONS; i++)
    {
        DrawUnScaledSprite (
            Explosions[i].x,
            Explosions[i].y,
            (W_GetNumForName(ExplosionInfo[Explosions[i].which].name) +
             (Explosions[i].frame>>1)),
            16
        );
    }
}

void DoTransmitterExplosion ( void )
{
    byte * back;
    int i;

    VL_ClearVideo(0);
    back=SafeMalloc(800*linewidth);

    CalcTics();
    CalcTics();
    DrawNormalSprite(0,0,W_GetNumForName("transmit"));
    PrepareBackground ( back );
    SetupTransmitterExplosions ();
    CacheTransmitterExplosions ();
    DrawBackground ( back );
    FlipPage();
    VL_FadeIn (0, 255, origpal, 30);
    SHAKETICS=VBLCOUNTER*15;
    for (i=0; i<(VBLCOUNTER*15); i+=tics)
    {
        DrawBackground ( back );
        DrawTransmitterExplosions ();
        FlipPage();
        CalcTics();
        UpdateTransmitterExplosions ();
    }
    VL_FadeOut (0, 255, 63, 63, 63, 150);
    screenfaded=false;
    SD_Play(SD_PLAYERTCSND);
    SD_Play(SD_PLAYERTBSND);
    SD_Play(SD_PLAYERDWSND);
    SD_Play(SD_PLAYERLNSND);
    SD_Play(SD_PLAYERIPFSND);
    VL_FadeOut (0, 255, 0, 0, 0, 30);
    TurnShakeOff();

    SafeFree(back);
}

void ShowTransmitter ( void )
{
    MenuFadeOut();
    DrawNormalSprite(0,0,W_GetNumForName("transmit"));
    FlipPage();
    VL_FadeIn (0, 255, origpal, 30);
    I_Delay(30);
    VL_FadeOut (0, 255, 0, 0, 0, 30);
}

void ShowFinalDoor ( void )
{
    byte pal[768];

    MenuFadeOut();

    VL_ClearBuffer (bufferofs, 0);
    DrawNormalSprite (0, (200-120)>>1, W_GetNumForName("finldoor"));
    FlipPage();
    memcpy(&pal[0],W_CacheLumpName("findrpal",PU_CACHE,CvtNull, 1),768);
    VL_NormalizePalette(&pal[0]);
    SD_Play(SD_OPENDOORSND);
    VL_FadeIn (0, 255, pal, 30);
    I_Delay(30);
    VL_FadeOut (0, 255, 0, 0, 0, 30);
}

void ShowFinalFire ( void )
{
    byte pal[768];

    MenuFadeOut();

    VL_ClearBuffer (bufferofs, 0);
    DrawNormalSprite (0, (200-120)>>1, W_GetNumForName("finlfire"));
    FlipPage();
    memcpy(&pal[0],W_CacheLumpName("finfrpal",PU_CACHE,CvtNull, 1),768);
    VL_NormalizePalette(&pal[0]);
    SD_Play(SD_BAZOOKAFIRESND);
    VL_FadeIn (0, 255, pal, 30);
    SD_Play(SD_FIREBOMBFIRESND);
    I_Delay(2);
    SD_Play(SD_HEATSEEKFIRESND);
    I_Delay(2);
    SD_Play(SD_DRUNKFIRESND);
    SD_Play(SD_HEATSEEKFIRESND);
    I_Delay(2);
    SD_Play(SD_ATKMP40SND);
    SD_Play(SD_ATKMP40SND);
    I_Delay(2);
    SD_Play(SD_HEATSEEKFIRESND);
    SD_Play(SD_FIREBOMBFIRESND);
    I_Delay(2);
    SD_Play(SD_ATKMP40SND);
    SD_Play(SD_HEATSEEKFIRESND);
    I_Delay(2);
    SD_Play(SD_DRUNKFIRESND);
    I_Delay(2);
    SD_Play(SD_FIREBOMBFIRESND);
    I_Delay(2);
    SD_Play(SD_HEATSEEKFIRESND);
    SD_Play(SD_ATKMP40SND);
    I_Delay(2);
    SD_Play(SD_DRUNKFIRESND);
    SD_Play(SD_HEATSEEKFIRESND);
    I_Delay(2);
    SD_Play(SD_FIREBOMBFIRESND);
    SD_Play(SD_ATKMP40SND);
    I_Delay(2);
    SD_Play(SD_DRUNKFIRESND);
    I_Delay(2);
    SD_Play(SD_HEATSEEKFIRESND);
    SD_Play(SD_FIREBOMBFIRESND);
    I_Delay(2);
    SD_Play(SD_ATKMP40SND);
    SD_Play(SD_HEATSEEKFIRESND);
    I_Delay(2);
    SD_Play(SD_DRUNKFIRESND);
    SD_Play(SD_FIREBOMBFIRESND);
    I_Delay(2);
    SD_Play(SD_HEATSEEKFIRESND);
    SD_Play(SD_DRUNKFIRESND);
    SD_Play(SD_BAZOOKAFIRESND);
    I_Delay(4);
    VL_FadeOut (0, 255, 0, 0, 0, 30);
}

void ScrollString ( int cy, char * string, byte * bkgnd, int scrolltime, int pausetime )
{
    int x,y;
    int width,height;
    int time1,time2;

    LastScan=0;
    US_MeasureStr (&width, &height, "%s", string);

    x=(320-width)>>1;
    y=cy-(height>>1);
    time1 = ((220 - y)*scrolltime)/(220+height);
    time2 = scrolltime-time1;

    WarpString (x, 210, x, y, time1, bkgnd, string);

    if (LastScan !=0)
        return;

    I_Delay(pausetime);

    if (LastScan !=0)
        return;

    WarpString (x, y, x, -10-height, time2, bkgnd, string);
}

void DoBurningCastle ( void )
{
    byte * back;

    LastScan=0;
    VL_ClearVideo(0);
    back=SafeMalloc(800*linewidth);

    DrawNormalSprite(0,0,W_GetNumForName("finale"));
    PrepareBackground ( back );
    CurrentFont = smallfont;
    FlipPage();
    VL_FadeIn (0, 255, origpal, 30);
    CurrentFont = (font_t *)W_CacheLumpName ("newfnt1", PU_STATIC, Cvt_font_t, 1 );
    ScrollString ( 150, &burnCastle1Msg[0], back, 4*VBLCOUNTER, 80);
    W_CacheLumpName ("newfnt1", PU_CACHE, Cvt_font_t, 1);
    VL_FadeOut (0, 255, 0, 0, 0, 80);
    SafeFree(back);
}

void DoFailedScreen ( void )
{
    byte * back;

    back=SafeMalloc(800*linewidth);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    PrepareBackground ( back );
    CurrentFont = smallfont;
    FlipPage();
    VL_FadeIn (0, 255, origpal, 30);
    ScrollString ( 100, &notDoneMsg[0], back, 4*VBLCOUNTER, 100);
    VL_FadeOut (0, 255, 0, 0, 0, 80);
    SafeFree(back);
}

void DoTryAgainScreen ( void )
{
    byte * back;

    back=SafeMalloc(800*linewidth);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    PrepareBackground ( back );
    CurrentFont = smallfont;
    FlipPage();
    VL_FadeIn (0, 255, origpal, 30);
    ScrollString ( 100, tryAgainMsg, back, 4*VBLCOUNTER, 80);
    VL_FadeOut (0, 255, 0, 0, 0, 80);
    SafeFree(back);
}

void ResetWorldExplosion ( ExplosionType * Explosion )
{
    Explosion->which=RandomNumber("Explosion",0)%NUMEXPLOSIONTYPES;
    Explosion->frame=0;
//   RandomNumber("Explosion",1)%ExplosionInfo[Explosions[i].which].numframes;
    Explosion->x=(RandomNumber("Explosion",2))+64;
    Explosion->y=(RandomNumber("Explosion",3)%180);
}

void SetupWorldExplosions ( void )
{
    int i;

    for (i=0; i<MAXTRANSMITTEREXPLOSIONS; i++)
    {
        ResetWorldExplosion(&Explosions[i]);
    }
}
void UpdateWorldExplosions ( void )
{
    int i;
    for (i=0; i<MAXTRANSMITTEREXPLOSIONS; i++)
    {
        Explosions[i].frame+=tics;
        if (Explosions[i].frame>=(ExplosionInfo[Explosions[i].which].numframes<<1))
        {
            ResetWorldExplosion(&Explosions[i]);
            SD_Play(SD_EXPLODEFLOORSND+(RandomNumber("Explosion",4)>>7));
        }
    }
}

void DestroyEarth ( void )
{
    byte * back;
    int i;

    VL_ClearVideo(0);
    back=SafeMalloc(800*linewidth);

    CalcTics();
    CalcTics();
    DrawNormalSprite(0,0,W_GetNumForName("ourearth"));
    PrepareBackground ( back );
    SetupWorldExplosions ();
    CacheTransmitterExplosions ();
    DrawBackground ( back );
    FlipPage();
    VL_FadeIn (0, 255, origpal, 30);
    SHAKETICS=VBLCOUNTER*10;
    for (i=0; i<(VBLCOUNTER*10); i+=tics)
    {
        DrawBackground ( back );
        DrawTransmitterExplosions ();
        FlipPage();
        CalcTics();
        UpdateWorldExplosions ();
    }
    VL_FadeOut (0, 255, 63, 63, 63, 150);
    screenfaded=false;
    if (gamestate.violence==vl_excessive)
        SD_Play(SD_YOUSUCKSND);
    VL_FadeOut (0, 255, 0, 0, 0, 50);
    TurnShakeOff();

    SafeFree(back);
}

boolean DestroyedAllEggs ( void )
{
    statobj_t * temp;

    for(temp=FIRSTSTAT; temp; temp=temp->statnext)
    {
        if (temp->itemnumber==stat_tomlarva)
            return false;
    }
    return true;
}

void DoSanNicolas ( void )
{
    byte pal[768];

    LastScan=0;
    VL_ClearVideo(0);
    DrawNormalSprite(0,16,W_GetNumForName("nicolas"));
    DrawNormalSprite(10,200-58,W_GetNumForName("budgcut"));
    FlipPage();
    memcpy(&pal[0],W_CacheLumpName("nicpal",PU_CACHE, CvtNull, 1),768);
    VL_NormalizePalette(&pal[0]);
    VL_FadeIn (0, 255, pal, 30);
    I_Delay(60);
    VL_FadeOut (0, 255, 0, 0, 0, 80);
}

void PlayerQuestionScreen ( void )
{
    byte * back;

    back=SafeMalloc(800*linewidth);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    PrepareBackground ( back );
    CurrentFont = smallfont;
    FlipPage();
    VL_FadeIn (0, 255, origpal, 30);
    CurrentFont = (font_t *)W_CacheLumpName ("newfnt1", PU_STATIC, Cvt_font_t, 1);
    ScrollString ( 100, &doneMsg[0], back, 4*VBLCOUNTER, 40);
    ScrollString ( 100, &youWin1Msg[0], back, 4*VBLCOUNTER, 50);
    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawXYPic ( 8, 100-24, W_GetNumForName("player2"));
    PrepareBackground ( back );
    CurrentFont = smallfont;
    SD_Play(SD_PLAYERTBSND);
    ScrollString ( 100, &youWin2Msg[0], back, 4*VBLCOUNTER, 100);
    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawXYPic ( 8, 100-24, W_GetNumForName("player1"));
    PrepareBackground ( back );
    SD_Play(SD_PLAYERTCSND);
    ScrollString ( 100, &youWin3Msg[0], back, 4*VBLCOUNTER, 100);
    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawXYPic ( 8, 100-24, W_GetNumForName("player4"));
    PrepareBackground ( back );
    SD_Play(SD_PLAYERLNSND);
    ScrollString ( 100, &youWin4Msg[0], back, 4*VBLCOUNTER, 100);
    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawXYPic ( 8, 100-24, W_GetNumForName("player3"));
    PrepareBackground ( back );
    SD_Play(SD_PLAYERDWSND);
    ScrollString ( 100, &youWin5Msg[0], back, 4*VBLCOUNTER, 100);
    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawXYPic ( 8, 100-24, W_GetNumForName("player5"));
    PrepareBackground ( back );
    SD_Play(SD_PLAYERIPFSND);
    ScrollString ( 100, &youWin6Msg[0], back, 4*VBLCOUNTER, 100);
    W_CacheLumpName ("newfnt1", PU_CACHE, Cvt_font_t, 1);
    VL_FadeOut (0, 255, 0, 0, 0, 80);
    SafeFree(back);
}

void DoYouWin ( void )
{
    pic_t * pic;
    byte * back;

    back=SafeMalloc(800*linewidth);
    LastScan=0;
    VL_ClearVideo(0);
    pic = (pic_t *) W_CacheLumpNum (W_GetNumForName ("mmbk"), PU_CACHE, Cvt_pic_t, 1);
    VWB_DrawPic (0, 0, pic);
    PrepareBackground ( back );
    FlipPage();
    VL_FadeIn (0, 255, origpal, 30);
    CurrentFont = (font_t *)W_CacheLumpName ("newfnt1", PU_STATIC, Cvt_font_t, 1);
    ScrollString ( 100, &youWin7Msg[0], back, 4*VBLCOUNTER, 300);
    W_CacheLumpName ("newfnt1", PU_CACHE, Cvt_font_t, 1);
    VL_FadeOut (0, 255, 0, 0, 0, 80);
    SafeFree(back);
}

void DoFinalEnd ( void )
{
    pic_t * pic;
    byte * back;

    back=SafeMalloc(800*linewidth);
    LastScan=0;
    VL_ClearVideo(0);
    pic = (pic_t *) W_CacheLumpNum (W_GetNumForName ("mmbk"), PU_CACHE, Cvt_pic_t, 1);
    VWB_DrawPic (0, 0, pic);
    DrawNormalSprite(0,0,W_GetNumForName("sombrero"));
    DrawNormalSprite(0,0,W_GetNumForName("amflag"));
    DrawNormalSprite(0,0,W_GetNumForName("witchhat"));
    DrawNormalSprite(0,0,W_GetNumForName("esterhat"));
    DrawNormalSprite(0,0,W_GetNumForName("santahat"));
    PrepareBackground ( back );
    FlipPage();
    VL_FadeIn (0, 255, origpal, 30);
    CurrentFont = (font_t *)W_CacheLumpName ("newfnt1", PU_STATIC, Cvt_font_t, 1);
    ScrollString ( 100, &youWin8Msg[0], back, 4*VBLCOUNTER, 100);
    W_CacheLumpName ("newfnt1", PU_CACHE, Cvt_font_t, 1);
    VL_FadeOut (0, 255, 0, 0, 0, 80);
    SafeFree(back);
    VL_ClearVideo(0);
}

static char     dipMsg[] =
    "The Developers of Incredible Power!\n"
    "\n"
    "Susan  Tom  Jim  Stephen\n"
    "Mark  William  Chuck\n";

static char     creditsMsg[] =
    "Rise of the Triad Credits\n";

static char     credits1Msg[] =
    "Programmers\n"
    "\n"
    "Mark Dochtermann\n"
    "William Scarboro\n"
    "Jim Dose'\n"
    "Nolan Martin\n";

static char     credits2Msg[] =
    "Creative Director\n"
    "\n"
    "Tom Hall\n";

static char     credits3Msg[] =
    "Artists\n"
    "\n"
    "Stephen Hornback\n"
    "Tim Neveu\n"
    "Chuck Jones\n"
    "Susan Singer\n"
    "James Storey\n"
    "Cygnus Multimedia\n";

static char     credits4Msg[] =
    "Level Designers\n"
    "\n"
    "Tom Hall\n"
    "Joseph Selinske\n"
    "Marianna Vayntrub\n"
    "Joe Siegler\n";
static char     credits5Msg[] =
    "Music\n"
    "\n"
    "Lee Jackson\n"
    "Bobby Prince\n";

static char     credits6Msg[] =
    "Robot Models\n"
    "\n"
    "Gregor Punchatz\n";

static char     credits7Msg[] =
    "Special Thanks\n"
    "\n"
    "George Broussard\n"
    "Scott Miller\n"
    "Steven Blackburn\n"
    "Apogee Technical Support\n"
    "Apogee Support Staff\n"
    "John Carmack\n"
    "Ken Silverman\n";

static char     credits8Msg[] =
    "The Hand of God\n"
    "\n"
    "Tim Neveu's Hand\n";

static char     credits9Msg[] =
    "Dog Snout and Paw\n"
    "\n"
    "Loki\n"
    "The Carpet Wetting Maestro\n";

static char     credits10Msg[] =
    "Krist's chair\n"
    "\n"
    "Stephen Blackburn's Comfy Chair\n"
    "Marianna's Paper and Glue\n";

static char     credits11Msg[] =
    "Character Voices\n"
    "\n"
    "Darian - Mark Dochtermann\n"
    "Krist - Joe Siegler\n"
    "NME - Sound CD#4005\n"
    "Oscuro - Tom Hall\n"
    "Low Guard - Steve Quarrella\n"
    "High Guard - Steven Blackburn\n"
    "Over Patrol - Chuck Jones\n";

static char     credits12Msg[] =
    "Character Voices Continued\n"
    "\n"
    "Strike Team - Scott Miller\n"
    "Lightning Guard - William Scarboro\n"
    "Triad Enforcer - George Broussard\n"
    "All Monks - Tom Hall\n"
    "Taradino - Joe Selinske\n"
    "Lorelei - Pau Suet Ying\n"
    "Ian Paul - Jim Dose'\n"
    "Doug - Lee Jackson\n"
    "Thi - Susan Singer\n";

static char     actorsMsg[] =
    "The Actors\n";

static char     actors1Msg[] =
    "Low Guard\n"
    "\n"
    "Steve Quarrella\n";

static char     actors2Msg[] =
    "High Guard\n"
    "\n"
    "Steven Blackburn\n";

static char     actors3Msg[] =
    "Over Patrol\n"
    "\n"
    "Nolan Martin\n";

static char     actors4Msg[] =
    "Strike Team\n"
    "\n"
    "Scott Miller\n";

static char     actors5Msg[] =
    "Lightning Guard\n"
    "\n"
    "Kevin Green\n";

static char     actors6Msg[] =
    "Triad Enforcer\n"
    "\n"
    "George Broussard\n";

static char     actors7Msg[] =
    "Death Monk\n"
    "\n"
    "Lee Jackson\n";

static char     actors8Msg[] =
    "Deathfire Monk\n"
    "\n"
    "Allen Blum III\n";

static char     actors9Msg[] =
    "Robot Guard\n"
    "\n"
    "Himself\n";

static char     actors10Msg[] =
    "General Darian\n"
    "\n"
    "Steve Maines\n";

static char     actors11Msg[] =
    "Sebastian Krist\n"
    "\n"
    "Joe Siegler\n";

static char     actors12Msg[] =
    "The NME\n"
    "\n"
    "Himself\n";

static char     actors13Msg[] =
    "El Oscuro\n"
    "\n"
    "Tom Hall\n";


static char     cut1Msg[] =
    "Deathfire Monk\n"
    "\n"
    "Mark Dochtermann\n";


static char     cut2Msg[] =
    "Over Patrol\n"
    "\n"
    "Pat Miller\n";


static char     cut3Msg[] =
    "Low Guard\n"
    "\n"
    "Marianna Vayntrub\n";


static char     cut4Msg[] =
    "Strike Team\n"
    "\n"
    "Ann Grauerholz\n";

static char     cut5Msg[] =
    "Lightning Guard\n"
    "\n"
    "William Scarboro\n";


static char     cut6Msg[] =
    "High Guard\n"
    "\n"
    "Stephen Hornback\n";



static char     playersCutMsg[] =
    "Actors who were\n"
    "cut from the game\n";

void DIPCredits ( void )
{
    byte * back;

    back=SafeMalloc(800*linewidth);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    PrepareBackground ( back );
    FlipPage();
    VL_FadeIn (0, 255, origpal, 30);
    CurrentFont = (font_t *)W_CacheLumpName ("newfnt1", PU_STATIC, Cvt_font_t, 1);
    ScrollString ( 100, &creditsMsg[0], back, 4*VBLCOUNTER, 30);
    CurrentFont = smallfont;
    ScrollString ( 100, &credits1Msg[0], back, 4*VBLCOUNTER, 50);
    ScrollString ( 100, &credits2Msg[0], back, 4*VBLCOUNTER, 50);
    ScrollString ( 100, &credits3Msg[0], back, 4*VBLCOUNTER, 50);
    ScrollString ( 100, &credits4Msg[0], back, 4*VBLCOUNTER, 50);
    ScrollString ( 100, &credits5Msg[0], back, 4*VBLCOUNTER, 50);
    ScrollString ( 100, &credits6Msg[0], back, 4*VBLCOUNTER, 50);
    ScrollString ( 100, &credits7Msg[0], back, 4*VBLCOUNTER, 50);
    ScrollString ( 100, &credits8Msg[0], back, 4*VBLCOUNTER, 50);
    ScrollString ( 100, &credits9Msg[0], back, 4*VBLCOUNTER, 50);
    ScrollString ( 100, &credits10Msg[0], back, 4*VBLCOUNTER, 50);
    ScrollString ( 100, &credits11Msg[0], back, 4*VBLCOUNTER, 80);
    ScrollString ( 100, &credits12Msg[0], back, 4*VBLCOUNTER, 80);

    CurrentFont = (font_t *)W_CacheLumpName ("newfnt1", PU_STATIC, Cvt_font_t, 1);
    ScrollString ( 100, &actorsMsg[0], back, 4*VBLCOUNTER, 50);

    CurrentFont = smallfont;
    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("lwgshoo2"));
    PrepareBackground ( back );
    ScrollString ( 100, &actors1Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("hg2shoo2"));
    PrepareBackground ( back );
    ScrollString ( 100, &actors2Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("obpshoo1"));
    PrepareBackground ( back );
    ScrollString ( 100, &actors3Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("ankshoo1"));
    PrepareBackground ( back );
    ScrollString ( 100, &actors4Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("ligrise4"));
    PrepareBackground ( back );
    ScrollString ( 100, &actors5Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("tritoss5"));
    PrepareBackground ( back );
    ScrollString ( 100, &actors6Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("monkdr4"));
    PrepareBackground ( back );
    ScrollString ( 100, &actors7Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("allksh4"));
    PrepareBackground ( back );
    ScrollString ( 100, &actors8Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("robogrd1"));
    PrepareBackground ( back );
    ScrollString ( 100, &actors9Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("darshoo1"));
    PrepareBackground ( back );
    ScrollString ( 100, &actors10Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("hdope8"));
    PrepareBackground ( back );
    ScrollString ( 100, &actors11Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("rbody101"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("rhead101"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("rsw01"));
    PrepareBackground ( back );
    ScrollString ( 100, &actors12Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("tomfly21"));
    PrepareBackground ( back );
    ScrollString ( 100, &actors13Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    PrepareBackground ( back );
    CurrentFont = (font_t *)W_CacheLumpName ("newfnt1", PU_STATIC, Cvt_font_t, 1);
    ScrollString ( 100, &playersCutMsg[0], back, 4*VBLCOUNTER, 40);

    CurrentFont = smallfont;
    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("cutmark"));
    PrepareBackground ( back );
    ScrollString ( 100, &cut1Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("cutpat"));
    PrepareBackground ( back );
    ScrollString ( 100, &cut2Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("cutmari"));
    PrepareBackground ( back );
    ScrollString ( 100, &cut3Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("cutann"));
    PrepareBackground ( back );
    ScrollString ( 100, &cut4Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("cutwill"));
    PrepareBackground ( back );
    ScrollString ( 100, &cut5Msg[0], back, 4*VBLCOUNTER, 50);

    VL_DrawPostPic (W_GetNumForName("trilogo"));
    DrawNormalSprite(0,(200-128)>>1,W_GetNumForName("cutstev"));
    PrepareBackground ( back );
    ScrollString ( 100, &cut6Msg[0], back, 4*VBLCOUNTER, 50);

    VL_FadeOut (0, 255, 0, 0, 0, 80);

    DrawNormalSprite(0,0,W_GetNumForName("grouppic"));
    PrepareBackground ( back );
    FlipPage();
    VL_FadeIn (0, 255, origpal, 30);
    ScrollString ( 175, &dipMsg[0], back, 4*VBLCOUNTER, 140);
    VL_FadeOut (0, 255, 0, 0, 0, 80);

    W_CacheLumpName ("newfnt1", PU_CACHE, Cvt_font_t, 1);
    SafeFree(back);
}

void DoEndCinematic ( void )
{
    EnableScreenStretch();

    viewwidth = 320;//MAXSCREENWIDTH;
    viewheight = 200;//MAXSCREENHEIGHT;
    MU_FadeOut ( 1000 );
    MU_StopSong ();

    ShowFinalDoor();
    ShowTransmitter ();
    ShowFinalFire();
    DoTransmitterExplosion();
    MU_StartSong(song_youwin);
    DoBurningCastle ();
    DoSanNicolas();
    if (DestroyedAllEggs () == true)
    {
        PlayerQuestionScreen();
        DIPCredits();
        DoYouWin();
        if (LastScan !=0)
        {
            IN_UpdateKeyboard();
            return;
        }
        DoFinalEnd();
    }
    else
    {
        MU_StartSong(song_gameover);
        DoFailedScreen();
        DestroyEarth();
        DoTryAgainScreen ();
        playstate=ex_warped;
        gamestate.mapon=33;
    }
    IN_UpdateKeyboard();
}

void DoInBetweenCinematic (int yoffset, int lump, int delay, char * string )
{
    int width,height;
    int x,y;

    VL_FadeOut (0, 255, 0, 0, 0, 20);
    VL_ClearBuffer (bufferofs, 0);
    DrawNormalSprite(0,yoffset,lump);

    CurrentFont=smallfont;
    US_MeasureStr (&width, &height, "%s", string);
    x=(320-width)>>1;
    y=190-height;
    US_ClippedPrint (x, y, string);
    FlipPage();
    VL_FadeIn(0,255,origpal,20);
    I_Delay (delay);
    VL_FadeOut (0, 255, 0, 0, 0, 20);
}
#endif


//******************************************************************************
//
// DoCreditScreen
//
//******************************************************************************

#define NUMFIRSTCREDITMESSAGES 22
#define NUMSECONDCREDITMESSAGES 28

typedef struct CreditType {
    char  text[80];
    byte  font;
    byte  endy;
} CreditType;

CreditType FirstCredits[NUMFIRSTCREDITMESSAGES] =
{
    {"Rise of the Triad Credits",0,0},
    {"COPYRIGHT (c) 1995 Apogee Software Ltd.",1,10},
    {"Apogee's Developers of Incredible Power",1,20},
    {"Creative Director",0,30},
    {"Tom Hall",1,40},
    {"Programmers",0,50},
    {"Mark Dochtermann  William Scarboro",1,60},
    {"Jim Dose'  Nolan Martin",1,66},
    {"Artists",0,76},
    {"Stephen Hornback  Chuck Jones",1,86},
    {"Susan Singer  Tim Neveu",1,92},
    {"James Storey  Cygnus Multimedia",1,98},
    {"Level Designers",0,108},
    {"Joseph Selinske  Tom Hall",1,118},
    {"Marianna Vayntrub  Joe Siegler",1,124},
    {"Musicians",0,134},
    {"Lee Jackson  Robert Prince",1,144},
    {"Uniforms",0,154},
    {"D.J. Goodwin  Matt McKinney",1,164},
    {"Special Thanks",0,174},
    {"John Carmack  Ken Silverman  Gregor Punchatz",1,184},
};

CreditType SecondCredits[NUMSECONDCREDITMESSAGES] =
{
    {"Rise of the Triad Credits",0,0},
    {"COPYRIGHT (c) 1995 Apogee Software Ltd.",1,10},
    {"Executive Producers",0,20},
    {"George Broussard  Scott Miller",1,30},
    {"Manual Design",0,40},
    {"Robert Atkins",1,50},
    {"Beta Testers",0,60},
    {"Steven Blackburn",1,70},
    {"Todd Aubin  Mike Bartelt",1,76},
    {"Wayne Benner  Neil Bonner",1,82},
    {"Glenn Brensinger  Douglas Brewer",1,88},
    {"David Butler  Daniel Creeron",1,94},
    {"Scott Darling  Jason Ewasiuk",1,100},
    {"Craig Hamilton  Ken Heckbert",1,106},
    {"Terry Herrin  Greg Hively",1,112},
    {"John Howard  Douglas Howell",1,118},
    {"Dennis Kurek  Hank Leukart",1,124},
    {"Jim Lietzan  Ken Mayer",1,130},
    {"Wayne Millard  Penny Plant",1,136},
    {"Brian Prinner  Jeff Rausch",1,142},
    {"Kelly Rogers  Neil Rubenking",1,148},
    {"Steven Salter  Chris White",1,154},
    {"Special Thanks",0,162},
    {"Apogee Technical Support  Pau Suet Ying",1,172},
    {"Anthony, Zach, Rajan, Miki, Loki",1,178},
    {"Nathan, Petro, Tim, Jake, MacKay",1,184},
    {"Loyal, Ric, Teller, Amano",1,190},
};

void DrawPreviousCredits ( int num, CreditType * Credits )
{
    int width;
    int height;
    int x,y;
    int i;

    for(i=0; i<num; i++)
    {
        if (Credits[i].font==0)
            CurrentFont=smallfont;
        else
            CurrentFont=tinyfont;
        US_MeasureStr (&width, &height, "%s", &(Credits[i].text[0]));
        x=(320-width)>>1;
        y=Credits[i].endy;
        US_ClippedPrint (x, y+4, &Credits[i].text[0]);
    }
}

#define CREDITSTARTY 220
//******************************************************************************
//
// WarpCreditString
//
//******************************************************************************

extern boolean dopefish;
void WarpCreditString ( int time, byte * back, int num, CreditType * Credits)
{
    int dy;
    int cy;
    int x;
    int y;
    int width;
    int height;
    boolean soundplayed;


    LastScan = 0;

    if (Credits[num].font==0)
        CurrentFont=smallfont;
    else
        CurrentFont=tinyfont;
    US_MeasureStr (&width, &height, "%s", &(Credits[num].text[0]));

    x=(320-width)>>1;
    y=Credits[num].endy;
    dy=((y-CREDITSTARTY)<<16)/time;
    cy=CREDITSTARTY<<16;

    CalcTics();

    soundplayed=false;

    while (time>0)
    {
        DrawBackground ( back );
        DrawPreviousCredits ( num, Credits );
        if (Credits[num].font==0)
            CurrentFont=smallfont;
        else
            CurrentFont=tinyfont;
        US_ClippedPrint (x, (cy>>16)+4, &Credits[num].text[0]);
        if ( ((cy>>16)<196) && (soundplayed==false))
        {
            if ((dopefish==true) && (SD_Started==true))
            {
                int snd;

                do
                {
                    snd=(RandomNumber("DoCredits",0)+RandomNumber("DoCredits",0))%MAXSOUNDS;
                }
                while (SD_SoundOkay ( snd ) == false);
                SD_Play ( snd );
            }
            else
            {
//            SD_Play ( SD_BAZOOKAFIRESND );
#if (SHAREWARE == 0)
                SD_Play ( SD_BAZOOKAFIRESND + (RandomNumber("DoCredits",1)%13) );
#else
                SD_Play ( SD_BAZOOKAFIRESND + (RandomNumber("DoCredits",1)%6) );
#endif
                soundplayed=true;
            }
        }
        FlipPage();
        CalcTics();
        cy+=dy*tics;
        time-=tics;
        if (LastScan != 0)
            break;
    }
}

void DoCreditScreen ( void )
{
    int trilogo;
    int time;
    byte * bkgnd;
    font_t * oldfont;
    int i;
    EnableScreenStretch();
    viewwidth = 320;//MAXSCREENWIDTH;
    viewheight = 200;//MAXSCREENHEIGHT;

    bkgnd=SafeMalloc(800*linewidth);

    trilogo=W_GetNumForName("trilogo");
    VL_DrawPostPic (trilogo);
//  SetTextMode (  );

    PrepareBackground ( bkgnd );

    oldfont=CurrentFont;

    for(i=0; i<NUMFIRSTCREDITMESSAGES; i++)
    {
        time = (CREDITSTARTY - FirstCredits[i].endy)*(VBLCOUNTER*1)/CREDITSTARTY;
//      time = VBLCOUNTER;
        WarpCreditString ( time, bkgnd, i, FirstCredits );
        IN_PumpEvents();
//      SD_Play ( SD_EXPLODESND );
        if (LastScan !=0)
            break;
    }
    i=NUMFIRSTCREDITMESSAGES;
    DrawBackground ( bkgnd );
    DrawPreviousCredits ( i, FirstCredits );
    FlipPage();
    IN_PumpEvents();

    I_Delay(40);

    for(i=0; i<NUMSECONDCREDITMESSAGES; i++)
    {
        time = (CREDITSTARTY - SecondCredits[i].endy)*(VBLCOUNTER/2)/CREDITSTARTY;
//      time = VBLCOUNTER;
        WarpCreditString ( time, bkgnd, i, SecondCredits );
        IN_PumpEvents();
//      SD_Play ( SD_EXPLODESND );
        if (LastScan !=0)
            break;
    }
    i=NUMSECONDCREDITMESSAGES;
    DrawBackground ( bkgnd );
    DrawPreviousCredits ( i, SecondCredits );
    FlipPage();
    IN_PumpEvents();

    I_Delay(40);
    MenuFadeOut();
    VL_ClearVideo (0);

    SafeFree(bkgnd);
    CurrentFont=oldfont;
}


#define NUMSTORYLINES 16

char * MicroStory[NUMSTORYLINES] =
{
    "You are a member of HUNT, the",
    "High-Risk United Nations Taskforce.",
    "Stranded on an island in the",
    "Pacific, you must battle a master",
    "of pyrotechnics, hundreds of",
    "members of a death cult, and their",
    "leader, El Oscuro.",
    "\0",
    "You must reach the transmitter",
    "that is signalling the systematic",
    "destruction of Los Angeles.",
    "\0",
    "If you fail, millions will die",
    "and you will be tortured.",
    "\0",
    "So, you know, don't fail."
};

void DoMicroStoryScreen ( void )
{
    pic_t * pic;
    int x,y;
    int i;

    VL_FadeOut (0, 255, 0, 0, 0, 20);

    pic=(pic_t *)W_CacheLumpName("mmbk",PU_CACHE,Cvt_pic_t,1);
    VWB_DrawPic (0, 0, pic);
    CheckHolidays();

    x=15;
    y=30;

    IFont = ( cfont_t * )W_CacheLumpName( "sifont", PU_CACHE, Cvt_cfont_t, 1);
    for(i=0; i<NUMSTORYLINES; i++)
    {
        DrawIntensityString (x, y, MicroStory[i], 241);
        y += 9;
    }

    FlipPage();
    MenuFadeIn();
    I_Delay (100);//240

    VL_FadeOut (0, 255, 0, 0, 0, 20);
}

void  DrawMenuPost (int height, byte * src, byte * buf)
{
    int frac = hp_startfrac;
    while (height--) {
        *buf = src[frac >> 16];

        buf += linewidth;
        frac += hp_srcstep;
    }
}

void  DrawMapPost (int height, byte * src, byte * buf)
{
    int frac = 0;
    while (height--) {
        *buf = src[frac >> 16];

        buf += linewidth;
        frac += hp_srcstep;
    }
}

void DrawRotRow(int count, byte * dest, byte * src)
{
    unsigned eax, ecx, edx;
//	unsigned a, b, c,d;
    byte * srctmp;
    byte * desttmp;

    ecx = mr_yfrac;
    edx = mr_xfrac;

    if ((iGLOBAL_SCREENWIDTH == 320)||(iG_masked==true))
    {
        while (count--) {
            eax = edx >> 16;
            if (eax < 256 && (ecx >> 16) < 512) {
                eax = (eax << 9) | ((ecx << 7) >> (32-9));
            } else {
                eax = 0;
            }

            *dest++ = src[eax];

            edx += mr_xstep;
            ecx += mr_ystep;
        }
    } else if (iGLOBAL_SCREENWIDTH == 640) {
        while (count--) {
            eax = edx >> 16;
            if (eax < (256*2.0) && (ecx >> 16) < (512*1.8)) {
                eax = (eax << 10) | ((ecx << 6) >> (32-10));
            } else {
                eax = 0;
            }

            *dest++ = src[eax];

            edx += mr_xstep;
            ecx += mr_ystep;
        }
    } else if (iGLOBAL_SCREENWIDTH == 800) {
        srctmp = src;
        desttmp = dest;

        desttmp -= (iGLOBAL_SCREENWIDTH*1);

        ecx = mr_yfrac;
        edx = mr_xfrac;
        //count = 800
//zxcv
        while (count--) {
            eax = edx >> 16;//edx=4146069504 eax=63264  edx/eax = 65536->0x10000

            //a=(eax << 9); //=eax*512
            //a=(eax << 7); //=eax*128
            //a=512|128; = 640;
            //SetTextMode (  );
            //a=(ecx >> 16);//ecx=4102225920 a=62595   ecx/65536 = 62595

            //         Y-dir                    x-dir
            if (eax < (256*2.5) && (ecx >> 16) < (512*2)) {
                //eax = (eax << 9) | ((ecx << 7) >> (32-9));
                eax = (eax << 10) | ((ecx << 6) >> (32-10));
                /*		eax = (eax * 512*2) ;
                							//(23)
                		eax += 	 ((ecx ) >> (32-9));//ecx=196608
                		*/
            } else {
                eax = 0;


            }
            //desttmp -= centeroffset;
            *desttmp++ = srctmp[eax];
            //*desttmp++ = srctmp[eax];

            edx += mr_xstep;
            ecx += mr_ystep;
        }

    }
    else if (iGLOBAL_SCREENWIDTH >= 1024) {
        srctmp = src;
        desttmp = dest;

        desttmp -= (iGLOBAL_SCREENWIDTH*1);

/*
        ecx = mr_yfrac;
        edx = mr_xfrac;
*/
	while (count--) {
            eax = edx >> 16;
            if (eax < (256*3.1) && (ecx >> 16) < (512*2.0)) {
                eax = (eax << 10) | ((ecx << 6) >> (32-10));
            } else {
		eax = 0;
            }
			
            *desttmp++ = srctmp[eax];
			
            edx += mr_xstep;
            ecx += mr_ystep;
        }
    }
    else
    {
        srctmp = src;
        desttmp = dest;

        desttmp -= (iGLOBAL_SCREENWIDTH*1);

/*
        ecx = mr_yfrac;
        edx = mr_xfrac;
*/
	while (count--) {
            eax = edx >> 16;
            if (eax < (256*4) && (ecx >> 16) < (512*4)) {
                eax = (eax << 10) | ((ecx << 6) >> (32-10));
            } else {
		eax = 0;
            }
			
            *desttmp++ = srctmp[eax];
			
            edx += mr_xstep;
            ecx += mr_ystep;
        }
    
    
    }
}

void DrawMaskedRotRow(int count, byte * dest, byte * src)
{
    unsigned eax;
    unsigned xfrac, yfrac;

    xfrac = mr_xfrac;
    yfrac = mr_yfrac;

    while (count--) {
        eax = xfrac >> 16;
        if (eax < 256 && (yfrac >> 16) < 512) {
            eax = (eax << 9) | ((yfrac << 7) >> (32-9));
        } else {
            eax = 0;
        }

        if (src[eax] != 0xff) *dest = src[eax];
        dest++;

        xfrac += mr_xstep;
        yfrac += mr_ystep;
    }
}

void DrawSkyPost (byte * buf, byte * src, int height)
{
    {
        int i = 0;
        byte *orig_src = src;
        // org code
        while (height--) {
            *buf = shadingtable[*src];

            buf += linewidth;
            src = orig_src + (++i*200/iGLOBAL_SCREENHEIGHT);
        }
        //
    }

    /*
    int lw = linewidth * 2;
    int h  = height;

    while (h--) {
    	*(buf) = shadingtable[*src];
    	buf += lw;
    	*(buf) = shadingtable[*src];
    	buf += lw;

    	//buf += lw;
    	src++;

    }*/
}

#define CEILINGCOLOR 24 //default color when no sky or floor
#define FLOORCOLOR 32

void RefreshClear (void)
{
    int start, base;

    memset(spotvis, 0, sizeof(spotvis));

    if (fandc) {
        return;
    }

    start = min(centery, viewheight);

    if (start > 0) {
        VL_Bar(0, 0, iGLOBAL_SCREENHEIGHT, start, CEILINGCOLOR);
    } else {
        start = 0;
    }

    base = start;

    start = min(viewheight-start, viewheight);
    if (start > 0) {
        VL_Bar(0, base, iGLOBAL_SCREENHEIGHT, start, FLOORCOLOR);
    }
}


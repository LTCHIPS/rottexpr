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
#include "rt_def.h"
#include "watcom.h"
#include <stdio.h>
#include <string.h>
#include "modexlib.h"
#include "rt_util.h"
#include "rt_draw.h"
#include "rt_scale.h"
#include "_rt_scal.h"
#include "rt_sc_a.h"
#include "engine.h"
#include "w_wad.h"
#include "z_zone.h"
#include "lumpy.h"
#include "rt_main.h"
#include "rt_ted.h"
#include "rt_vid.h"
#include "rt_view.h"
#include "rt_playr.h"

/*
=============================================================================

                               GLOBALS

=============================================================================
*/

// Draw Column vars

int dc_texturemid;
int dc_iscale;
int dc_invscale;
int sprtopoffset;
int dc_yl;
int dc_yh;
//byte * dc_firstsource;
byte * dc_source;
int centeryclipped;
int transparentlevel=0;

/*
==========================
=
= SetPlayerLightLevel
=
==========================
*/

void SetPlayerLightLevel (void)
{
    int i;
    int lv;
    int intercept;
    int height;

    whereami=23;
    if (MISCVARS->GASON==1)
    {
        shadingtable=greenmap+(MISCVARS->gasindex<<8);
        return;
    }

    if (fulllight || fog)
    {
        shadingtable=colormap+(1<<12);
        return;
    }

    height=PLAYERHEIGHT;

    if (player->angle < FINEANGLES/8 || player->angle > 7*FINEANGLES/8)
        intercept=(player->x>>11)&0x1c;
    else if (player->angle < 3*FINEANGLES/8)
        intercept=(player->y>>11)&0x1c;
    else if (player->angle < 5*FINEANGLES/8)
        intercept=(player->x>>11)&0x1c;
    else
        intercept=(player->y>>11)&0x1c;

    if (lightsource)
    {
        lv=(((LightSourceAt(player->x>>16,player->y>>16)>>intercept)&0xf)>>1);
        i=maxshade-(height>>normalshade)-lv;
        if (i<minshade) i=minshade;
        shadingtable=colormap+(i<<8);
    }
    else
    {
        i=maxshade-(height>>normalshade);
        if (i<minshade) i=minshade;
        shadingtable=colormap+(i<<8);
    }
}



/*
==========================
=
= SetLightLevel
=
==========================
*/

void SetLightLevel (int height)
{
    int i;

    whereami=24;
    if (MISCVARS->GASON==1)
    {
        shadingtable=greenmap+(MISCVARS->gasindex<<8);
        return;
    }

    if (fulllight)
    {
        shadingtable=colormap+(1<<12);
        return;
    }
    if (fog)
    {
        i=((height*200/iGLOBAL_SCREENHEIGHT)>>normalshade)+minshade;
        if (i>maxshade) i=maxshade;
        shadingtable=colormap+(i<<8);
    }
    else
    {
        i=maxshade-(height>>normalshade);
        if (i<minshade) i=minshade;
        shadingtable=colormap+(i<<8);
    }
}

/*
==========================
=
= ScaleTransparentPost
=
==========================
*/
void ScaleTransparentPost (byte * src, byte * buf, int level)
{
    int  offset;
    int  length;
    int  topscreen;
    int  bottomscreen;
    byte * oldlevel;
    byte * seelevel;

    whereami=25;

    seelevel=colormap+(((level+64)>>2)<<8);
    oldlevel=shadingtable;
    offset=*(src++);
    for (; offset!=255;)
    {
        length=*(src++);
        topscreen = sprtopoffset + (dc_invscale*offset);
        bottomscreen = topscreen + (dc_invscale*length);
        dc_yl = (topscreen+SFRACUNIT)>>SFRACBITS;
        dc_yh = ((bottomscreen-1)>>SFRACBITS);
        if (dc_yh >= viewheight)
            dc_yh = viewheight-1;
        if (dc_yl < 0)
            dc_yl = 0;
        if ((*src)==254)
        {
            shadingtable=seelevel;
            if (dc_yl <= dc_yh)
                R_TransColumn (buf);
            src++;
            offset=*(src++);
            shadingtable=oldlevel;
        }
        else
        {
            if (dc_yl <= dc_yh)
            {
                dc_source=src-offset;
                R_DrawColumn (buf);
            }
            src+=length;
            offset=*(src++);
        }
    }

    whereami=-2;
}


void ScaleMaskedPost (byte * src, byte * buf)
{
    int  offset;
    int  length;
    int  topscreen;
    int  bottomscreen;

    whereami=26;
    offset=*(src++);
    for (; offset!=255;)
    {
        length=*(src++);
        topscreen = sprtopoffset + (dc_invscale*offset);
        bottomscreen = topscreen + (dc_invscale*length);
        dc_yl = (topscreen+SFRACUNIT)>>SFRACBITS;
        dc_yh = ((bottomscreen-1)>>SFRACBITS);
        if (dc_yh >= viewheight)
            dc_yh = viewheight-1;
        if (dc_yl < 0)
            dc_yl = 0;
        if (dc_yl <= dc_yh)
        {
            dc_source=src-offset;
            R_DrawColumn (buf);
        }
        src+=length;
        offset=*(src++);
    }
}

void ScaleClippedPost (byte * src, byte * buf)
{
    int  offset;
    int  length;
    int  topscreen;
    int  bottomscreen;

    whereami=27;
    offset=*(src++);
    for (; offset!=255;)
    {
        length=*(src++);
        topscreen = sprtopoffset + (dc_invscale*offset);
        bottomscreen = topscreen + (dc_invscale*length);
        dc_yl = (topscreen+SFRACUNIT-1)>>SFRACBITS;
        dc_yh = ((bottomscreen-1)>>SFRACBITS);
        if (dc_yh >= viewheight)
            dc_yh = viewheight-1;
        if (dc_yl < 0)
            dc_yl = 0;
        if (dc_yl <= dc_yh)
        {
            dc_source=src-offset;
            R_DrawClippedColumn (buf);
        }
        src+=length;
        offset=*(src++);
    }
}

void ScaleSolidMaskedPost (int color, byte * src, byte * buf)
{
    int  offset;
    int  length;
    int  topscreen;
    int  bottomscreen;

    whereami=28;
    offset=*(src++);
    for (; offset!=255;)
    {
        length=*(src++);
        topscreen = sprtopoffset + (dc_invscale*offset);
        bottomscreen = topscreen + (dc_invscale*length);
        dc_yl = (topscreen+SFRACUNIT)>>SFRACBITS;
        dc_yh = ((bottomscreen-1)>>SFRACBITS);
        if (dc_yh >= viewheight)
            dc_yh = viewheight-1;
        if (dc_yl < 0)
            dc_yl = 0;
        if (dc_yl <= dc_yh)
        {
            dc_source=src-offset;
            R_DrawSolidColumn (color, buf);
        }
        src+=length;
        offset=*(src++);
    }

}


void ScaleTransparentClippedPost (byte * src, byte * buf, int level)
{
    int  offset;
    int  length;
    int  topscreen;
    int  bottomscreen;
    byte * oldlevel;
    byte * seelevel;

    whereami=29;

    seelevel=colormap+(((level+64)>>2)<<8);
    oldlevel=shadingtable;
    offset=*(src++);
    for (; offset!=255;)
    {
        length=*(src++);
        topscreen = sprtopoffset + (dc_invscale*offset);
        bottomscreen = topscreen + (dc_invscale*length);
        dc_yl = (topscreen+SFRACUNIT)>>SFRACBITS;
        dc_yh = ((bottomscreen-1)>>SFRACBITS);
        if (dc_yh >= viewheight)
            dc_yh = viewheight-1;
        if (dc_yl < 0)
            dc_yl = 0;
        if ((*src)==254)
        {
            shadingtable=seelevel;
            if (dc_yl <= dc_yh)
                R_TransColumn (buf);
            src++;
            offset=*(src++);
            shadingtable=oldlevel;
        }
        else
        {
            if (dc_yl <= dc_yh)
            {
                dc_source=src-offset;
                R_DrawClippedColumn (buf);
            }
            src+=length;
            offset=*(src++);
        }
    }

}


void ScaleMaskedWidePost (byte * src, byte * buf, int x, int width)
{
    buf += x;

    while (width--) {
        ScaleMaskedPost(src,buf);
        buf++;
    }
}

void ScaleClippedWidePost (byte * src, byte * buf, int x, int width)
{
    buf += x;

    while (width--) {
        ScaleClippedPost(src,buf);
        buf++;
    }
}


/*
=======================
=
= ScaleShape
=
=======================
*/

void ScaleShape (visobj_t * sprite)
{
    byte *shape;
    int      frac;
    patch_t *p;
    int      x1,x2;
    int      tx;
    int      size;

    whereami=32;
    shape=W_CacheLumpNum(sprite->shapenum,PU_CACHE, Cvt_patch_t, 1);
    p=(patch_t *)shape;
    size=p->origsize>>7;
//   sprite->viewheight<<=1;
    dc_invscale=sprite->viewheight<<((10-HEIGHTFRACTION)-size);
    tx=-p->leftoffset;
    sprite->viewx=(sprite->viewx<<SFRACBITS)-(sprite->viewheight<<(SFRACBITS-HEIGHTFRACTION-1))+(SFRACUNIT>>1);
//
// calculate edges of the shape
//
    x1 = (sprite->viewx+(tx*dc_invscale))>>SFRACBITS;
    if (x1 >= viewwidth)
    {
        return;               // off the right side
    }
    tx+=p->width;
    x2 = ((sprite->viewx+(tx*dc_invscale)) >>SFRACBITS) - 1 ;
    if (x2 < 0)
    {
        return;         // off the left side
    }

// dc_iscale=(1<<(16+6+HEIGHTFRACTION+size))/sprite->viewheight;
    dc_iscale=0xffffffffu/(unsigned)dc_invscale;
    dc_texturemid=(((sprite->h1<<size) + p->topoffset)<<SFRACBITS);//+(SFRACUNIT>>1);
    sprtopoffset=centeryfrac -  FixedMul(dc_texturemid,dc_invscale);
    shadingtable=sprite->colormap;

    if (x1<0)
    {
        frac=dc_iscale*(-x1);
        x1=0;
    }
    else
        frac=0;
    x2 = x2 >= viewwidth ? viewwidth-1 : x2;

    if (sprite->viewheight>((1<<(HEIGHTFRACTION+6))<<size))
    {
        int      texturecolumn;
        int      lastcolumn;
        int      startx;
        int      width;

        width=1;
        startx=0;
        lastcolumn=-1;
        for (; x1<=x2 ; x1++, frac += dc_iscale)
        {
            if (posts[x1].wallheight>sprite->viewheight)
            {
                if (lastcolumn>=0)
                {
                    ScaleMaskedWidePost(((p->collumnofs[lastcolumn])+shape),(byte *)bufferofs,startx,width);
                    width=1;
                    lastcolumn=-1;
                }
                continue;
            }
            texturecolumn = frac>>SFRACBITS;
            if ((texturecolumn==lastcolumn)&&(width<9))
            {
                width++;
                continue;
            }
            else
            {
                if (lastcolumn>=0)
                {
                    ScaleMaskedWidePost(((p->collumnofs[lastcolumn])+shape),(byte *)bufferofs,startx,width);
                    width=1;
                    startx=x1;
                    lastcolumn=texturecolumn;
                }
                else
                {
                    startx=x1;
                    lastcolumn=texturecolumn;
                }
            }
        }
        if (lastcolumn!=-1)
            ScaleMaskedWidePost(((p->collumnofs[lastcolumn])+shape),(byte *)bufferofs,startx,width);
    }
    else
    {
        byte * b;
        int    startfrac;
        int    startx;

        startx=x1;
        startfrac=frac;
        if (doublestep>1)
        {
            {
                frac=startfrac;
                for (x1=startx; x1<=x2; x1+=2, frac += (dc_iscale<<1))
                {
                    if (
                        (posts[x1].wallheight>sprite->viewheight) &&
                        (posts[x1+1].wallheight>sprite->viewheight)
                    )
                        continue;
                    if (x1==viewwidth-1)
                        ScaleMaskedWidePost(((p->collumnofs[frac>>SFRACBITS])+shape),(byte *)bufferofs,x1,1);
                    else
                        ScaleMaskedWidePost(((p->collumnofs[frac>>SFRACBITS])+shape),(byte *)bufferofs,x1,2);
                }
            }
        }
        else
        {
            {
                frac=startfrac;

                b=(byte *)bufferofs+startx;
                
                for (x1=startx; x1<=x2; x1++, frac += dc_iscale,b++)
                {
                    if (posts[x1].wallheight>sprite->viewheight)
                        continue;
                    ScaleMaskedPost(((p->collumnofs[frac>>SFRACBITS])+shape),b);
                }
            }
        }
    }
}


/*
=======================
=
= ScaleTranparentShape
=
=======================
*/

void ScaleTransparentShape (visobj_t * sprite)
{
    byte *shape;
    int      frac;
    transpatch_t *p;
    int      x1,x2;
    int      tx;
    int      size;
    byte * b;
    int    startfrac;
    int    startx;

    whereami=33;
    shape=W_CacheLumpNum(sprite->shapenum,PU_CACHE, Cvt_transpatch_t, 1);
    p=(transpatch_t *)shape;
    size=p->origsize>>7;
    dc_invscale=sprite->viewheight<<((10-HEIGHTFRACTION)-size);
    tx=-p->leftoffset;
    sprite->viewx=(sprite->viewx<<SFRACBITS)-(sprite->viewheight<<(SFRACBITS-HEIGHTFRACTION-1));
//
// calculate edges of the shape
//
    x1 = (sprite->viewx+(tx*dc_invscale))>>SFRACBITS;
    if (x1 >= viewwidth)
    {
        return;               // off the right side
    }
    tx+=p->width;
    x2 = ((sprite->viewx+(tx*dc_invscale)) >>SFRACBITS) - 1 ;
    if (x2 < 0)
    {
        return;         // off the left side
    }

//   dc_iscale=(1<<(16+6+HEIGHTFRACTION+size))/sprite->viewheight;
    dc_iscale=0xffffffffu/(unsigned)dc_invscale;
    dc_texturemid=(((sprite->h1<<size)+p->topoffset)<<SFRACBITS);//+(SFRACUNIT>>1);
    sprtopoffset=centeryfrac - FixedMul(dc_texturemid,dc_invscale);
    shadingtable=sprite->colormap;

    if (x1<0)
    {
        frac=dc_iscale*(-x1);
        x1=0;
    }
    else
        frac=0;
    x2 = x2 >= viewwidth ? viewwidth-1 : x2;

    startx=x1;
    startfrac=frac;

    {
        frac=startfrac;
        b=(byte *)bufferofs+startx;

        for (x1=startx; x1<=x2; x1++, frac += dc_iscale,b++)
        {
            if (posts[x1].wallheight>sprite->viewheight)
                continue;
            ScaleTransparentPost(((p->collumnofs[frac>>SFRACBITS])+shape),b,sprite->h2);
        }
    }
}

/*
=======================
=
= ScaleSolidShape
=
=======================
*/

void ScaleSolidShape (visobj_t * sprite)
{
    byte *shape;
    int      frac;
    patch_t *p;
    int      x1,x2;
    int      tx;
    int      size;
    byte * b;
    int    startfrac;
    int    startx;

    whereami=34;
    shape=W_CacheLumpNum(sprite->shapenum,PU_CACHE, Cvt_patch_t, 1);
    p=(patch_t *)shape;
    size=p->origsize>>7;
    dc_invscale=sprite->viewheight<<((10-HEIGHTFRACTION)-size);
    tx=-p->leftoffset;
    sprite->viewx=(sprite->viewx<<SFRACBITS)-(sprite->viewheight<<(SFRACBITS-HEIGHTFRACTION-1))+(SFRACUNIT>>1);
//
// calculate edges of the shape
//
    x1 = (sprite->viewx+(tx*dc_invscale))>>SFRACBITS;
    if (x1 >= viewwidth)
    {
        return;               // off the right side
    }
    tx+=p->width;
    x2 = ((sprite->viewx+(tx*dc_invscale)) >>SFRACBITS) - 1 ;
    if (x2 < 0)
    {
        return;         // off the left side
    }

//   dc_iscale=(1<<(16+6+HEIGHTFRACTION+size))/sprite->viewheight;
    dc_iscale=0xffffffffu/(unsigned)dc_invscale;
    dc_texturemid=(((sprite->h1<<size)+p->topoffset)<<SFRACBITS);//+(SFRACUNIT>>1);
    sprtopoffset=centeryfrac - FixedMul(dc_texturemid,dc_invscale);
    shadingtable=sprite->colormap;

    if (x1<0)
    {
        frac=dc_iscale*(-x1);
        x1=0;
    }
    else
        frac=0;
    x2 = x2 >= viewwidth ? viewwidth-1 : x2;

    startx=x1;
    startfrac=frac;

    {
        frac=startfrac;

        b=(byte *)bufferofs+startx;

        for (x1=startx; x1<=x2; x1++, frac += dc_iscale,b++)
        {
            if (posts[x1].wallheight>sprite->viewheight)
                continue;
            ScaleSolidMaskedPost(sprite->h2,((p->collumnofs[frac>>SFRACBITS])+shape),b);
        }
    }
}


/*
=======================
=
= ScaleWeapon
=
=======================
*/

void ScaleWeapon (int xoff, int y, int shapenum)
{
    byte *shape;
    int      frac;
    int      h;
    patch_t *p;
    int      x1,x2;
    int      tx;
    int      xcent;
    byte * b;
    int    startfrac;
    int    startx;

    whereami=35;
    SetPlayerLightLevel();
    shape=W_CacheLumpNum(shapenum,PU_CACHE, Cvt_patch_t, 1);
    p=(patch_t *)shape;
    h=((p->origsize*weaponscale)>>17);
    centeryclipped=(viewheight-h)+FixedMul(y,weaponscale);
    xcent=centerx+FixedMul(xoff,weaponscale);
    dc_invscale=(h<<17)/p->origsize;

    tx=-p->leftoffset;
    xcent=(xcent<<SFRACBITS)-(h<<SFRACBITS);
//
// calculate edges of the shape
//
    x1 = (xcent+(tx*dc_invscale))>>SFRACBITS;
    if (x1 >= viewwidth)
        return;               // off the right side
    tx+=p->width;
    x2 = ((xcent+(tx*dc_invscale)) >>SFRACBITS) - 1 ;
    if (x2 < 0)
        return;         // off the left side

    dc_iscale=0xffffffffu/(unsigned)dc_invscale;
    dc_texturemid=(((p->origsize>>1)+p->topoffset)<<SFRACBITS)+(SFRACUNIT>>2);
    sprtopoffset=(centeryclipped<<16) - FixedMul(dc_texturemid,dc_invscale);

//
// store information in a vissprite
//
    if (x1<0)
    {
        frac=dc_iscale*(-x1);
        x1=0;
    }
    else
        frac=0;

    x2 = x2 >= viewwidth ? viewwidth-1 : x2;

    startx=x1;
    startfrac=frac;
    {
        frac=startfrac;
        b=(byte *)bufferofs+startx;
        for (x1=startx; x1<=x2 ; x1++, frac += dc_iscale,b++)
            ScaleClippedPost(((p->collumnofs[frac>>SFRACBITS])+shape),b);
    }
}




/*
=======================
=
= DrawUnScaledSprite
=
=======================
*/

void DrawUnScaledSprite (int x, int y, int shapenum, int shade)
{
    byte *shape;
    int      frac;
    patch_t *p;
    int      x1,x2;
    int      tx;
    int      xcent;
    byte * b;
    int    startfrac;
    int    startx;

    whereami=36;
    shadingtable=colormap+(shade<<8);
    centeryclipped=y;
    xcent=x;
    shape=W_CacheLumpNum(shapenum,PU_CACHE, Cvt_patch_t, 1);
    p=(patch_t *)shape;
    dc_invscale=0x10000;

    tx=-p->leftoffset;
    xcent-=p->origsize>>1;
//
// calculate edges of the shape
//
    x1 = xcent+tx;
    if (x1 >= viewwidth)
        return;               // off the right side
    tx+=p->width;
    x2 = xcent+tx - 1;
    if (x2 < 0)
        return;         // off the left side

    dc_iscale=0x10000;
    dc_texturemid=(((p->height>>1)+p->topoffset)<<SFRACBITS);//+(SFRACUNIT>>1);
    sprtopoffset=(centeryclipped<<16) - dc_texturemid;

//
// store information in a vissprite
//
    if (x1<0)
    {
        frac=dc_iscale*(-x1);
        x1=0;
    }
    else
        frac = 0;

    x2 = x2 >= viewwidth ? viewwidth-1 : x2;

    startx=x1;
    startfrac=frac;
    
    {
        frac=startfrac;
        b=(byte *)bufferofs+startx;
        for (x1=startx; x1<=x2 ; x1++, frac += dc_iscale,b++)
            ScaleClippedPost(((p->collumnofs[frac>>SFRACBITS])+shape),b);
    }
}


/*
=======================
=
= DrawScreenSprite
=
=======================
*/

void DrawScreenSprite (int x, int y, int shapenum)
{
    whereami=37;
    ScaleWeapon (x-160, y-200, shapenum);
}

/*
=======================
=
= DrawPositionedScaledSprite
=
=======================
*/

void DrawPositionedScaledSprite (int x, int y, int shapenum, int height, int type)
{
    byte *shape;
    int      frac;
    patch_t *p;
    transpatch_t *tp;
    int      x1,x2;
    int      tx;
    int      xcent;
    byte * b;
    int    startfrac;
    int    startx;
    int    size;

    whereami=38;
    shadingtable=colormap+(1<<12);
    centeryclipped=y;
    xcent=x;
    shape=W_CacheLumpNum(shapenum,PU_CACHE, Cvt_patch_t, 1); // was transpatch, fixed
    p=(patch_t *)shape;
    tp=(transpatch_t *)shape;

    size=p->origsize>>7;
    dc_invscale=height<<(10-size);

    tx=-p->leftoffset;
    xcent=(xcent<<SFRACBITS)-(height<<(SFRACBITS-1));

//
// calculate edges of the shape
//
    x1 = (xcent+(tx*dc_invscale))>>SFRACBITS;
    if (x1 >= viewwidth)
        return;               // off the right side
    tx+=p->width;
    x2 = ((xcent+(tx*dc_invscale)) >>SFRACBITS) - 1 ;
    if (x2 < 0)
        return;         // off the left side

    dc_iscale=0xffffffffu/(unsigned)dc_invscale;
//   dc_iscale=(1<<(16+6+size))/height;
    dc_texturemid=(((32<<size)+p->topoffset)<<SFRACBITS)+(SFRACUNIT>>1);
    sprtopoffset=(centeryclipped<<16) - FixedMul(dc_texturemid,dc_invscale);

//
// store information in a vissprite
//
    if (x1<0)
    {
        frac=dc_iscale*(-x1);
        x1=0;
    }
    else
        frac=0;

    x2 = x2 >= viewwidth ? viewwidth-1 : x2;

    startx=x1;
    startfrac=frac;

    {
        frac=startfrac;
        b=(byte *)bufferofs+startx;

        for (x1=startx; x1<=x2 ; x1++, frac += dc_iscale,b++)
            if (type==0)
                ScaleClippedPost(((p->collumnofs[frac>>SFRACBITS])+shape),b);
            else
                ScaleTransparentClippedPost(((tp->collumnofs[frac>>SFRACBITS])+shape),b,transparentlevel);
    }
}


/*
=================
=
= DrawScreenSizedSprite
=
=================
*/
extern int G_gmasklump;
void DrawScreenSizedSprite (int lump)
{
    //draws gasmask among other things zxcv
    byte *shape,*src;
    int      frac;
    patch_t *p;
    int      x1,x2;
    int      tx;
//   int      plane;
    byte * b;
    int    startfrac;

    int  offset;
    int  length;
    int  topscreen;
    int  bottomscreen;
    byte  *cnt,*Ycnt;

    // SetTextMode (  );
    whereami=39;
    shadingtable=colormap+(1<<12);
    shape=W_CacheLumpNum(lump,PU_CACHE, Cvt_patch_t, 1);
    p=(patch_t *)shape;
    dc_invscale=(viewwidth<<16)/p->origsize;
    tx=-p->leftoffset;
    centeryclipped=viewheight>>1;
    //centeryclipped=(viewheight>>1)+43;
//
// calculate edges of the shape
//
    x1 = (tx*dc_invscale)>>SFRACBITS;
    if (x1 >= viewwidth)
    {
        return;               // off the right side
    }
    tx+=p->width;
    x2 = ((tx*dc_invscale) >>SFRACBITS) - 1 ;
    if (x2 < 0)
    {
        return;         // off the left side
    }

    dc_iscale=0xffffffffu/(unsigned)dc_invscale;
    dc_texturemid=(((p->origsize>>1) + p->topoffset)<<SFRACBITS)+(SFRACUNIT>>1);
    sprtopoffset=(centeryclipped<<16) -  FixedMul(dc_texturemid,dc_invscale);

    x2 = (viewwidth-1);

    startfrac=0;

    {
        frac=startfrac;
        b=(byte *)bufferofs;

        /////////////  BNA PATCH //////////////////////////////////////////////////////////
        //gmasklump=W_GetNumForName("p_gmask"); //=783
        //perhaps I should have painted the mask in a seperate buffer
        //and streched it and copyet it back, but that would demand
        //a new buffer (size=800x600) and slowed the game down so ?

        // if its the gasmask, paint black patches at hires
        if ((lump == G_gmasklump)&&(iGLOBAL_SCREENWIDTH>320)) {
            src = ((p->collumnofs[frac>>SFRACBITS])+shape);
            offset=*(src++);
            length=*(src++);
            topscreen = sprtopoffset + (dc_invscale*offset);
            bottomscreen = topscreen + (dc_invscale*length);
            dc_yl = (topscreen+SFRACUNIT-1)>>SFRACBITS;//=41  viewheight=584
            dc_yh = ((bottomscreen-1)>>SFRACBITS);//=540      viewwidth =800
            //paint upper black patch in gasmask
            for (cnt=b; cnt<b+viewwidth; cnt++) {
                for (Ycnt=cnt; Ycnt<cnt+(dc_yl*iGLOBAL_SCREENWIDTH); Ycnt+=iGLOBAL_SCREENWIDTH) {
                    *Ycnt = 36;
                }
            }
            //paint lower black patch in gasmask
            for (cnt=b+(dc_yh*iGLOBAL_SCREENWIDTH); cnt<b+(dc_yh*iGLOBAL_SCREENWIDTH)+viewwidth; cnt++) {
                for (Ycnt=cnt; Ycnt<b+(viewheight*iGLOBAL_SCREENWIDTH); Ycnt+=iGLOBAL_SCREENWIDTH) {
                    *Ycnt = 36;
                }
            }
        }
        ///////////////////////////////////////////////////////////////////////////////////
        for (x1=0; x1<=x2; x1++, frac += dc_iscale,b++)

        {
            ScaleClippedPost(((p->collumnofs[frac>>SFRACBITS])+shape),b);
        }
    }
}

//******************************************************************************
//
// DrawNormalPost
//
//******************************************************************************

void DrawNormalPost (byte * src, byte * buf)
{
    int  offset;
    int  length;
    int  s;

    whereami=40;

    while (1)
    {
        offset=*(src++);
        if (offset==0xff)
            return;
        else
        {
            length=*(src++);
            for (s=0; s<length; s++) {
                // Spaced out a little for tracking a bug. Should be equivalent.
                byte *saddr = src+s;
                byte *daddr = buf + ylookup[offset + s];
                byte val = *saddr;
                *daddr = val;
//                *(buf+ylookup[offset+s])=*(src+s);
            }
            src+=length;
        }
    }
}



//******************************************************************************
//
// DrawNormalSprite
//
//******************************************************************************

void DrawNormalSprite (int x, int y, int shapenum)
{
    byte *buffer;
    int cnt;
    byte *shape;
    patch_t *p;
    byte * b;
    int startx;

    whereami=41;

    shape = W_CacheLumpNum (shapenum, PU_CACHE, Cvt_patch_t, 1);
    p = (patch_t *)shape;

    if (((x-p->leftoffset)<0) || ((x-p->leftoffset+p->width)>iGLOBAL_SCREENWIDTH))
        Error ("DrawNormalSprite: x is out of range x=%d\n",x-p->leftoffset+p->width);
    if (((y-p->topoffset)<0) || ((y-p->topoffset+p->height)>iGLOBAL_SCREENHEIGHT))
        Error ("DrawNormalSprite: y is out of range y=%d\n",y-p->topoffset+p->height);

    startx=x-p->leftoffset;
    buffer = (byte*)bufferofs+ylookup[y-p->topoffset];
    {
        b=buffer+startx;
        for (cnt = 0; cnt < p->width; cnt++,b++)
            DrawNormalPost ((byte *)(p->collumnofs[cnt]+shape), b);
    }
}

void R_DrawColumn (byte * buf)
{
    // This is *NOT* 100% correct - DDOI
    int count;
    int frac, fracstep;
    byte *dest;

    count = dc_yh - dc_yl + 1;
    if (count < 0) return;

    dest = buf + ylookup[dc_yl];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;

    while (count--) {
        //*dest = test++;
        *dest = shadingtable[dc_source[(frac>>SFRACBITS)]];
        dest += iGLOBAL_SCREENWIDTH;
        frac += fracstep;
    }
}

void R_TransColumn (byte * buf)
{
    int count;
    byte *dest;

    count = dc_yh - dc_yl + 1;
    if (count < 0) return;

    dest = buf + ylookup[dc_yl];

    while (count--)
    {
        *dest = shadingtable[*dest];
        dest += iGLOBAL_SCREENWIDTH;
    }
}

void R_DrawWallColumn (byte * buf)
{
    // This is *NOT* 100% correct - DDOI
    int count;
    int frac, fracstep;
    byte *dest;

    count = dc_yh - dc_yl;
    if (count < 0) return;

    dest = buf + ylookup[dc_yl];

    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centery)*fracstep;
    frac <<= 10;
    fracstep <<= 10;

    while (count--) {
        //*dest = 6;
        *dest = shadingtable[dc_source[(((unsigned)frac)>>26)]];
        dest += iGLOBAL_SCREENWIDTH;
        frac += fracstep;
    }
}

void R_DrawClippedColumn (byte * buf)
{
    // This is *NOT* 100% correct - DDOI zxcv
    int count;
    int frac, fracstep;
    byte *dest;
//		byte *b;int y;

    count = dc_yh - dc_yl + 1;
    if (count < 0) return;

    dest = buf + ylookup[dc_yl];


    fracstep = dc_iscale;
    frac = dc_texturemid + (dc_yl-centeryclipped)*fracstep;

    while (count--) {
        *dest = shadingtable[dc_source[(((unsigned)frac)>>SFRACBITS)]];
        dest += iGLOBAL_SCREENWIDTH;
        frac += fracstep;
    }
}

void R_DrawSolidColumn (int color, byte * buf)
{
    int count;
    byte *dest;

    count = dc_yh - dc_yl + 1;
    if (count < 0) return;

    dest = buf + ylookup[dc_yl];

    while (count--)
    {
        *dest = (byte)color;
        dest += iGLOBAL_SCREENWIDTH;
    }
}

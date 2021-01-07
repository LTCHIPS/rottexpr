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
// W_wad.c

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "rt_def.h"
#include "rt_util.h"
#include "_w_wad.h"
#include "w_wad.h"
#include "z_zone.h"
#include "isr.h"
#include "develop.h"

#include "rt_crc.h"
#include "rt_main.h"

//=============
// GLOBALS
//=============

int             numlumps;
void            **lumpcache;

//=============
// STATICS
//=============

static lumpinfo_t      *lumpinfo;              // location of each lump on disk

/*
============================================================================

                                                LUMP BASED ROUTINES

============================================================================
*/

/*
====================
=
= W_AddFile
=
= All files are optional, but at least one file must be found
= Files with a .wad extension are wadlink files with multiple lumps
= Other files are single lumps with the base filename for the lump name
=
====================
*/

void W_AddFile (char *_filename)
{
    wadinfo_t               header;
    lumpinfo_t              *lump_p;
    unsigned                i;
    int                     handle, length;
    int                     startlump;
    filelump_t              *fileinfo, singleinfo, *allocedinfo = NULL;

    char filename[MAX_PATH];
    char buf[MAX_PATH+100];//bna++

    strncpy(filename, _filename, sizeof (filename));
    filename[sizeof (filename) - 1] = '\0';
    FixFilePath(filename);

    //bna section start
    if (access (filename, 0) != 0) {
        strcpy (buf,"Error, Could not find User file '");
        strcat (buf,filename);
        strcat (buf,"', ignoring file");
        printf("%s", buf);
    }
    //bna section end

//
// read the entire file in
//      FIXME: shared opens

    if ( (handle = open (filename,O_RDONLY | O_BINARY)) == -1)
        return;

    startlump = numlumps;

    if ( (strcmpi (filename+strlen(filename)-3, "wad" ) ) &&
            (strcmpi (filename+strlen(filename)-3, "rts" ) ) )
    {
        // single lump file
        if (!quiet)
            printf("    Adding single file %s.\n",filename);
        fileinfo = &singleinfo;
        singleinfo.filepos = 0;
        singleinfo.size = LONG(filelength(handle));
        ExtractFileBase (filename, singleinfo.name);
        numlumps++;
    }
    else
    {
        // WAD file
        if (!quiet)
            printf("    Adding %s.\n",filename);
        read (handle, &header, sizeof(header));
        if (strncmp(header.identification,"IWAD",4))
            Error ("Wad file %s doesn't have IWAD id\n",filename);
        header.numlumps = IntelLong(LONG(header.numlumps));
        header.infotableofs = IntelLong(LONG(header.infotableofs));
        length = header.numlumps*sizeof(filelump_t);
        fileinfo = malloc (length);
        if (!fileinfo)
            Error ("Wad file could not allocate header info");
        allocedinfo = fileinfo;
        lseek (handle, header.infotableofs, SEEK_SET);
        read (handle, fileinfo, length);

        numlumps += header.numlumps;
    }

//
// Fill in lumpinfo
//
    Z_Realloc((void **)&lumpinfo,numlumps*sizeof(lumpinfo_t));
//        lumpinfo = realloc (lumpinfo, numlumps*sizeof(lumpinfo_t));
//        if (!lumpinfo)
//           Error("W_AddFile: Could not realloc %ld bytes",numlumps*sizeof(lumpinfo_t));
    lump_p = &lumpinfo[startlump];

    for (i=startlump ; i<(unsigned int)numlumps ; i++,lump_p++, fileinfo++)
    {
        fileinfo->filepos = IntelLong(LONG(fileinfo->filepos));
        fileinfo->size = IntelLong(LONG(fileinfo->size));
        lump_p->handle = handle;
        lump_p->position = LONG(fileinfo->filepos);
        lump_p->size = LONG(fileinfo->size);
        strncpy (lump_p->name, fileinfo->name, 8);
    }

    free (allocedinfo);
}



/*
====================
=
= W_CheckWADIntegrity
=
====================
*/

void W_CheckWADIntegrity ( void )
{
// CRC disabled because it's not very useful these days
}



/*
====================
=
= W_InitMultipleFiles
=
= Pass a null terminated list of files to use.
=
= All files are optional, but at least one file must be found
=
= Files with a .wad extension are idlink files with multiple lumps
=
= Other files are single lumps with the base filename for the lump name
=
= Lump names can appear multiple times. The name searcher looks backwards,
= so a later file can override an earlier one.
=
====================
*/

void W_InitMultipleFiles (char **filenames)
{
//
// open all the files, load headers, and count lumps
//
    numlumps = 0;
    lumpinfo = SafeMalloc(5);   // will be realloced as lumps are added

    for ( ; *filenames ; filenames++)
        W_AddFile (*filenames);

    if (!numlumps)
        Error ("W_InitFiles: no files found");

//
// set up caching
//
    lumpcache = calloc (numlumps, sizeof(*lumpcache));
    if (!lumpcache)
        Error("W_InitFiles: lumpcache malloc failed size=%d\n",numlumps<<2);

    if (!quiet)
        printf("W_Wad: Wad Manager Started NUMLUMPS=%ld\n",(long int)numlumps);
        W_CheckWADIntegrity ();
}




/*
====================
=
= W_InitFile
=
= Just initialize from a single file
=
====================
*/

void W_InitFile (char *filename)
{
    char    *names[2];

    names[0] = filename;
    names[1] = NULL;
    W_InitMultipleFiles (names);
}



/*
====================
=
= W_NumLumps
=
====================
*/

int     W_NumLumps (void)
{
    return numlumps;
}



/*
====================
=
= W_CheckNumForName
=
= Returns -1 if name not found
=
====================
*/

int     W_CheckNumForName (char *name)
{
    char    name8[9];
    int             v1,v2;
    lumpinfo_t      *lump_p;
    lumpinfo_t      *endlump;

// make the name into two integers for easy compares

    strncpy (name8,name,8);
    name8[8] = 0;                   // in case the name was a fill 8 chars
    strupr (name8);                 // case insensitive

    v1 = *(int *)name8;
    v2 = *(int *)&name8[4];


// scan backwards so patch lump files take precedence

    lump_p = lumpinfo;
    endlump = lumpinfo + numlumps;

    while (lump_p != endlump)
    {
        if ( *(int *)lump_p->name == v1 && *(int *)&lump_p->name[4] == v2)
            return lump_p - lumpinfo;
        lump_p++;
    }


    return -1;
}


/*
====================
=
= W_GetNumForName
=
= Calls W_CheckNumForName, but bombs out if not found
=
====================
*/

int     W_GetNumForName (char *name)
{
    int     i;

    i = W_CheckNumForName (name);
    if (i != -1)
        return i;

    Error ("W_GetNumForName: %s not found!",name);
    return -1;
}


/*
====================
=
= W_LumpLength
=
= Returns the buffer size needed to load the given lump
=
====================
*/

int W_LumpLength (int lump)
{
    if (lump >= numlumps)
        Error ("W_LumpLength: %i >= numlumps",lump);
    return lumpinfo[lump].size;
}

/*
====================
=
= W_GetNameForNum
=
====================
*/

char *  W_GetNameForNum (int i)
{

    if (i>=numlumps)
        Error ("W_GetNameForNum: %i >= numlumps",i);
    return &(lumpinfo[i].name[0]);
}

/*
====================
=
= W_ReadLump
=
= Loads the lump into the given buffer, which must be >= W_LumpLength()
=
====================
*/
int readinglump;
byte * lumpdest;
void W_ReadLump (int lump, void *dest)
{
    int                     c;
    lumpinfo_t      *l;

    readinglump=lump;
    lumpdest=dest;
    if (lump >= numlumps)
        Error ("W_ReadLump: %i >= numlumps",lump);
    if (lump < 0)
        Error ("W_ReadLump: %i < 0",lump);
    l = lumpinfo+lump;

    lseek (l->handle, l->position, SEEK_SET);
    c = read (l->handle, dest, l->size);
    if (c < l->size)
        Error ("W_ReadLump: only read %i of %i on lump %i",c,l->size,lump);
}


/*
====================
=
= W_WriteLump
=
= Writes the lump into the given buffer, which must be >= W_LumpLength()
=
====================
*/

void W_WriteLump (int lump, void *src)
{
    int        c;
    lumpinfo_t *l;

    if (lump >= numlumps)
        Error ("W_WriteLump: %i >= numlumps",lump);
    if (lump < 0)
        Error ("W_WriteLump: %i < 0",lump);
    l = lumpinfo+lump;

    lseek (l->handle, l->position, SEEK_SET);
    c = write (l->handle, src, l->size);
    if (c < l->size)
        Error ("W_WriteLump: only wrote %i of %i on lump %i",c,l->size,lump);
}


/*
====================
=
= W_CacheLumpNum
=
====================
*/
void    *W_CacheLumpNum (int lump, int tag, converter_t converter, int numrec)
{
    if (lump >= (int)numlumps)
        Error ("W_CacheLumpNum: %i >= numlumps",lump);

    else if (lump < 0)
        Error ("W_CacheLumpNum: %i < 0  Taglevel: %i",lump,tag);

    if (!lumpcache[lump])
    {
        // read the lump in
        Z_Malloc (W_LumpLength (lump), tag, &lumpcache[lump]);
        W_ReadLump (lump, lumpcache[lump]);
        Debug("Invoking endian converter on %p, %i records\n", lumpcache[lump], numrec);
        converter(lumpcache[lump], numrec);
    }
    else
    {
        Z_ChangeTag (lumpcache[lump],tag);
    }

    return lumpcache[lump];
}


/*
====================
=
= W_CacheLumpName
=
====================
*/

void    *W_CacheLumpName (char *name, int tag, converter_t converter, int numrec)
{
    return W_CacheLumpNum (W_GetNumForName(name), tag, converter, numrec);
}

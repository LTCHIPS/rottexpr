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
// Z_zone.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "rt_def.h"
#include "_z_zone.h"
#include "z_zone.h"
#include "rt_util.h"
#include "develop.h"
#include "rt_net.h"

//MED
#include "memcheck.h"

int lowmemory=0;

/*
==============================================================================

                                                ZONE MEMORY ALLOCATION

There is never any space between memblocks, and there will never be two
contiguous free memblocks.

The rover can be left pointing at a non-empty block

It is of no value to free a cachable block, because it will get overwritten
automatically if needed

==============================================================================
*/

//Globals

int zonememorystarted=0;

// Statics

static memzone_t       *mainzone;
static memzone_t       *levelzone;
static int levelzonesize=LEVELZONESIZE;

/*
========================
=
= Z_ClearZone
=
========================
*/

void Z_ClearZone (memzone_t *zone)
{
    memblock_t      *block;

// set the entire zone to one free block

    zone->blocklist.next = zone->blocklist.prev = block =
                               (memblock_t *)( (byte *)zone + sizeof(memzone_t) );
    zone->blocklist.user = (void *)zone;
    zone->blocklist.tag = PU_STATIC;
    zone->rover = block;

    block->prev = block->next = &zone->blocklist;
    block->user = NULL;     // free block
    block->size = zone->size - sizeof(memzone_t);
}


/*
========================
=
= Z_AllocateZone
=
========================
*/

memzone_t *Z_AllocateZone (int size)
{
    memzone_t       *header;

    header = malloc (size+sizeof(memzone_t));
    if (!header)
        Error ("Z_AllocateZone: Couldn't malloc %zd bytes avail=%d\n",
               size+sizeof(memzone_t), Z_AvailHeap());
    header->size = size;
    Z_ClearZone (header);
    return header;
}

/*
========================
=
= Z_Init
=
========================
*/
void Z_Init (int size, int min)
{
    int maxsize;
    int sz;

    if (zonememorystarted==1)
        return;
    zonememorystarted=1;

    sz = GamePacketSize();

    sz*=MAXCMDS;

    if (ConsoleIsServer() == true)
        levelzonesize+=((numplayers*2)+1)*sz;
    else
        levelzonesize+=(numplayers+1)*sz;

    maxsize=((int)(Z_AvailHeap())-size-levelzonesize);
    if (maxsize<min)
    {
        UL_DisplayMemoryError (min-maxsize);
    }
    if (maxsize>MAXMEMORYSIZE)
        maxsize=(MAXMEMORYSIZE-levelzonesize);

    mainzone = Z_AllocateZone (maxsize);
    levelzone = Z_AllocateZone (levelzonesize);

    if (!quiet)
        printf("Z_INIT: %ld bytes\n",(long int)(maxsize+levelzonesize));

    if (maxsize<(min+(min>>1)))
    {
        lowmemory = 1;

        printf("==============================================================================\n");
        printf("WARNING: You are running ROTT with very little memory.  ROTT runs best with\n");
        printf("8 Megabytes of memory and no TSR's loaded in memory.  If you can free up more\n");
        printf("memory for ROTT then you should press CTRL-BREAK at this time. If you are\n");
        printf("unable to do this you will experience momentary pauses in game-play whenever\n");
        printf("you enter new areas of the game as well as an overall decreased performance.\n");
        printf("                        Press any key to continue\n");
        printf("==============================================================================\n");
        getch();
    }
}

/*
========================
=
= Z_ShutDown
=
========================
*/

void Z_ShutDown( void )
{
    if (zonememorystarted==0)
        return;
    zonememorystarted=0;
    free(mainzone);
    free(levelzone);
}

/*
========================
=
= Z_GetSize
=
========================
*/

int Z_GetSize (void *ptr)
{
    memblock_t      *block;

    block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
    return (block->size - sizeof(memblock_t));
}


/*
========================
=
= Z_Free
=
========================
*/

void Z_Free (void *ptr)
{
    memblock_t      *block, *other;

    block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
    if (!block->user)
        Error ("Z_Free: freed a freed pointer");

    if (block->user > (void **)0x100)       // smaller values are not pointers
        *block->user = 0;               // clear the user's mark
    block->user = NULL;     // mark as free

    other = block->prev;
    if (!other->user)
    {   // merge with previous free block
        other->size += block->size;
        other->next = block->next;
        other->next->prev = other;
        if (block == mainzone->rover)
            mainzone->rover = other;
        else if (block == levelzone->rover)
            levelzone->rover = other;
        block = other;
    }

    other = block->next;
    if (!other->user)
    {   // merge the next free block onto the end
        block->size += other->size;
        block->next = other->next;
        block->next->prev = block;
        if (other == mainzone->rover)
            mainzone->rover = block;
        else if (other == levelzone->rover)
            levelzone->rover = block;
    }
}


/*
========================
=
= Z_Malloc
=
= You can pass a NULL user if the tag is < PU_PURGELEVEL
========================
*/

void *Z_Malloc (int size, int tag, void *user)
{
    int             extra;
    memblock_t      *start, *rover, *new, *base;

//
// scan through the block list looking for the first free block
// of sufficient size, throwing out any purgable blocks along the way
//
//        size += sizeof(memblock_t);     // account for size of block header
    size = (size + sizeof(memblock_t) + 3)&~3;     // account for size of block header


//
// if there is a free block behind the rover, back up over them
//
    base = mainzone->rover;
    if (!base->prev->user)
        base = base->prev;

    rover = base;
    start = base->prev;

    do
    {
        if (rover == start)     // scaned all the way around the list
        {
            SoftError("OHSHIT\n");
            Z_DumpHeap(0,200);
            Error ("Z_Malloc: failed on allocation of %i bytes",size);
        }
        if (rover->user)
        {
            if (rover->tag < PU_PURGELEVEL)
                // hit a block that can't be purged, so move base past it
                base = rover = rover->next;
            else
            {
                // free the rover block (adding the size to base)
                base = base->prev;      // the rover can be the base block
                Z_Free ((byte *)rover+sizeof(memblock_t));
                base = base->next;
                rover = base->next;
            }
        }
        else
            rover = rover->next;
    } while (base->user || base->size < size);

//
// found a block big enough
//
    extra = base->size - size;
    if (extra >  MINFRAGMENT)
    {   // there will be a free fragment after the allocated block
        new = (memblock_t *) ((byte *)base + size );
        new->size = extra;
        new->user = NULL;               // free block
        new->tag = 0;
        new->prev = base;
        new->next = base->next;
        new->next->prev = new;
        base->next = new;
        base->size = size;
    }

    if (user)
    {
        base->user = user;                      // mark as an in use block
        *(void **)user = (void *) ((byte *)base + sizeof(memblock_t));
    }
    else
    {
        if (tag >= PU_PURGELEVEL)
            Error ("Z_Malloc: an owner is required for purgable blocks");
        base->user = (void *)2;         // mark as in use, but unowned
    }
    base->tag = tag;

    mainzone->rover = base->next;   // next allocation will start looking here

    return (void *) ((byte *)base + sizeof(memblock_t));
}

/*
========================
=
= Z_LevelMalloc
=
= Only use this for level structures.
= You can pass a NULL user if the tag is < PU_PURGELEVEL
========================
*/

void *Z_LevelMalloc (int size, int tag, void *user)
{
    int             extra;
    memblock_t      *start, *rover, *new, *base;

//
// scan through the block list looking for the first free block
// of sufficient size, throwing out any purgable blocks along the way
//
//        size += sizeof(memblock_t);     // account for size of block header
    size = (size + sizeof(memblock_t) + 3)&~3;     // account for size of block header


//
// if there is a free block behind the rover, back up over them
//
    base = levelzone->rover;
    if (!base->prev->user)
        base = base->prev;

    rover = base;
    start = base->prev;

    do
    {
        if (rover == start)     // scaned all the way around the list
        {
            SoftError("OHSHIT\n");
            Z_DumpHeap(0,200);
            Error ("Z_LevelMalloc: failed on allocation of %i bytes",size);
        }
        if (rover->user)
        {
            if (rover->tag < PU_PURGELEVEL)
                // hit a block that can't be purged, so move base past it
                base = rover = rover->next;
            else
            {
                // free the rover block (adding the size to base)
                base = base->prev;      // the rover can be the base block
                Z_Free ((byte *)rover+sizeof(memblock_t));
                base = base->next;
                rover = base->next;
            }
        }
        else
            rover = rover->next;
    } while (base->user || base->size < size);

//
// found a block big enough
//
    extra = base->size - size;
    if (extra >  MINFRAGMENT)
    {   // there will be a free fragment after the allocated block
        new = (memblock_t *) ((byte *)base + size );
        new->size = extra;
        new->user = NULL;               // free block
        new->tag = 0;
        new->prev = base;
        new->next = base->next;
        new->next->prev = new;
        base->next = new;
        base->size = size;
    }

    if (user)
    {
        base->user = user;                      // mark as an in use block
        *(void **)user = (void *) ((byte *)base + sizeof(memblock_t));
    }
    else
    {
        if (tag >= PU_PURGELEVEL)
            Error ("Z_Malloc: an owner is required for purgable blocks");
        base->user = (void *)2;         // mark as in use, but unowned
    }
    base->tag = tag;

    levelzone->rover = base->next;   // next allocation will start looking here

    return (void *) ((byte *)base + sizeof(memblock_t));
}



/*
========================
=
= Z_FreeTags
=
========================
*/

void Z_FreeTags (int lowtag, int hightag)
{
    memblock_t      *block, *next;

    for (block = mainzone->blocklist.next ; block != &mainzone->blocklist
            ; block = next)
    {
        next = block->next;             // get link before freeing
        if (!block->user)
            continue;                       // free block
        if (block->tag >= lowtag && block->tag <= hightag)
            Z_Free ( (byte *)block+sizeof(memblock_t));
    }
    for (block = levelzone->blocklist.next ; block != &levelzone->blocklist
            ; block = next)
    {
        next = block->next;             // get link before freeing
        if (!block->user)
            continue;                       // free block
        if (block->tag >= lowtag && block->tag <= hightag)
            Z_Free ( (byte *)block+sizeof(memblock_t));
    }
}

/*
========================
=
= Z_DumpHeap
=
========================
*/

void Z_DumpHeap (int lowtag, int hightag)
{
    memblock_t      *block;
    int             totalsize;

    SoftError("MAIN ZONE\n");
    SoftError("zone size: %i  location: %p\n",mainzone->size,mainzone);
    SoftError("tag range: %i to %i\n",lowtag, hightag);

    totalsize=0;

    for (block = mainzone->blocklist.next ; ; block = block->next)
    {
        if (block->tag >= lowtag && block->tag <= hightag)
        {
            SoftError("block:%p    size:%7i    user:%p    tag:%3i\n",
                      block, block->size, block->user, block->tag);
            totalsize+=block->size;
        }

        if (block->next == &mainzone->blocklist) {
            break;                  // all blocks have been hit
        }
        if ( (byte *)block + block->size != (byte *)block->next) {
            SoftError("ERROR: block size does not touch the next block\n");
        }
        if ( block->next->prev != block) {
            SoftError("ERROR: next block doesn't have proper back link\n");
        }
        if (!block->user && !block->next->user) {
            SoftError("ERROR: two consecutive free blocks\n");
        }
    }
    SoftError("Total Size of blocks = %d\n",totalsize);

    SoftError("LEVEL ZONE\n");
    SoftError("zone size: %i  location: %p\n",levelzone->size,levelzone);
    SoftError("tag range: %i to %i\n",lowtag, hightag);

    totalsize=0;

    for (block = levelzone->blocklist.next ; ; block = block->next)
    {
        if (block->tag >= lowtag && block->tag <= hightag)
        {
            SoftError("block:%p    size:%7i    user:%p    tag:%3i\n",
                      block, block->size, block->user, block->tag);
            totalsize+=block->size;
        }

        if (block->next == &levelzone->blocklist)
            break;                  // all blocks have been hit
        if ( (byte *)block + block->size != (byte *)block->next) {
            SoftError("ERROR: block size does not touch the next block\n");
        }
        if ( block->next->prev != block) {
            SoftError("ERROR: next block doesn't have proper back link\n");
        }
        if (!block->user && !block->next->user) {
            SoftError("ERROR: two consecutive free blocks\n");
        }
    }
    SoftError("Total Size of blocks = %d\n",totalsize);

}

/*
========================
=
= Z_UsedHeap
=
========================
*/

int Z_UsedHeap ( void )
{
    memblock_t      *block;
    int heapsize;


    heapsize=0;
    for (block = mainzone->blocklist.next ; ; block = block->next)
    {
        if ((block->tag>0) && (block->user>(void **)0))
            heapsize+=(block->size);
        if (block->next == &mainzone->blocklist)
            break;                  // all blocks have been hit
    }
    return heapsize;
}

/*
========================
=
= Z_UsedLevelHeap
=
========================
*/

int Z_UsedLevelHeap ( void )
{
    memblock_t      *block;
    int heapsize;


    heapsize=0;
    for (block = levelzone->blocklist.next ; ; block = block->next)
    {
        if ((block->tag>0) && (block->user>(void **)0))
            heapsize+=(block->size);
        if (block->next == &levelzone->blocklist)
            break;                  // all blocks have been hit
    }
    return heapsize;
}


/*
========================
=
= Z_UsedStaticHeap
=
========================
*/

int Z_UsedStaticHeap ( void )
{
    memblock_t      *block;
    int heapsize;


    heapsize=0;
    for (block = mainzone->blocklist.next ; ; block = block->next)
    {
        if ((block->tag>0) && (block->tag<PU_PURGELEVEL) && (block->user>(void **)0))
            heapsize+=(block->size);
        if (block->next == &mainzone->blocklist)
            break;                  // all blocks have been hit
    }
    return heapsize;
}


/*
========================
=
= Z_HeapSize
=
========================
*/

int Z_HeapSize ( void )
{
    return mainzone->size;
}


/*
========================
=
= Z_CheckHeap
=
========================
*/

void Z_CheckHeap (void)
{
    memblock_t      *block;

    // Check mainzone

    for (block = mainzone->blocklist.next ; ; block = block->next)
    {
        if (block->next == &mainzone->blocklist)
            break;                  // all blocks have been hit
        if ( (byte *)block + block->size != (byte *)block->next)
            Error ("Z_CheckHeap: block size does not touch the next block\n");
        if ( block->next->prev != block)
            Error ("Z_CheckHeap: next block doesn't have proper back link\n");
        if (!block->user && !block->next->user)
            Error ("Z_CheckHeap: two consecutive free blocks\n");
    }

    // Check levelzone

    for (block = levelzone->blocklist.next ; ; block = block->next)
    {
        if (block->next == &levelzone->blocklist)
            break;                  // all blocks have been hit
        if ( (byte *)block + block->size != (byte *)block->next)
            Error ("Z_CheckHeap: block size does not touch the next block\n");
        if ( block->next->prev != block)
            Error ("Z_CheckHeap: next block doesn't have proper back link\n");
        if (!block->user && !block->next->user)
            Error ("Z_CheckHeap: two consecutive free blocks\n");
    }
}


/*
========================
=
= Z_ChangeTag
=
========================
*/

void Z_ChangeTag (void *ptr, int tag)
{
    memblock_t      *block;

    block = (memblock_t *) ( (byte *)ptr - sizeof(memblock_t));
    block->tag = tag;
}

/*
========================
=
= Z_AvailHeap
=
========================
*/

int Z_AvailHeap ( void )
{
    return MAXMEMORYSIZE;
}

/*
========================
=
= Z_Realloc
=
========================
*/

void Z_Realloc (void ** ptr, int newsize)
{
    memblock_t      *block;
    void * newptr;
    int oldsize;

    block = (memblock_t *) ( (byte *)(*ptr) - sizeof(memblock_t));
    oldsize = block->size;
    newptr = SafeMalloc(newsize);
    if (oldsize > newsize)
    {
        oldsize = newsize;
    }
    memcpy( newptr, *ptr, oldsize );
    SafeFree( *ptr );
    *ptr = newptr;
}


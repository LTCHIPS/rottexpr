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
#ifndef _rt_com_private
#define _rt_com_private


#define SYNCPACKETSIZE 32

#define SYNC_PHASE0 6
#define SYNC_PHASE1 0
#define SYNC_PHASE2 1
#define SYNC_PHASE3 2
#define SYNC_PHASE4 3
#define SYNC_PHASE5 4
#define SYNC_MEMO   99
#define NUMSYNCPHASES 5

#define SYNCTIME    15

typedef struct
{
    byte type;
    int  phase;
    int  clocktime;
    int  delta;
    byte data[SYNCPACKETSIZE];
} syncpackettype;

typedef struct
{
    int  sendtime;
    int  deltatime;
    syncpackettype pkt;
} synctype;

#endif

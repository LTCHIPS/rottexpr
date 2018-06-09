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
#include "splib.h"
#include "rt_draw.h"
#include "rt_playr.h"
#include "isr.h"
#include <stdlib.h>
#include "rt_spbal.h"
#include "_rt_spba.h"
#include "sbconfig.h"
#include "rt_main.h"
#include "rt_map.h"

#include "rt_debug.h"
#include "rt_game.h"
#include "rt_str.h"
#include "rt_vid.h"
#include "rt_playr.h"
#include "rt_actor.h"
#include "rt_main.h"
#include "rt_util.h"
#include "rt_draw.h"
#include "rt_in.h"
#include "z_zone.h"
#include "rt_ted.h"
#include "rt_view.h"
#include "develop.h"
#include "version.h"
#include "scriplib.h"
#include <stdlib.h>

//MED
#include "memcheck.h"

/* This isn't of much use in Linux. */

void PollSpaceBall (void)
{
    STUB_FUNCTION;
}

void OpenSpaceBall (void)
{
    STUB_FUNCTION;
}

void CloseSpaceBall (void)
{
}

unsigned GetSpaceBallButtons (void)
{
    STUB_FUNCTION;

    return 0;
}


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
/**********************************************************************
   module: SNDCARDS.H

   author: James R. Dose
   date:   March 31, 1994

   Contains enumerated type definitions for sound cards.

   (c) Copyright 1994 James R. Dose.  All Rights Reserved.
**********************************************************************/

#ifndef __SNDCARDS_H
#define __SNDCARDS_H

#define ASS_VERSION_STRING "1.12"

typedef enum
   {
//   ASS_NoSound,
   SoundBlaster,
   ProAudioSpectrum,
   SoundMan16,
   Adlib,
   GenMidi,
   SoundCanvas,
   Awe32,
   WaveBlaster,
   SoundScape,
   UltraSound,
   SoundSource,
   TandySoundSource,
   PC,
   NumSoundCards
   } soundcardnames;

#endif

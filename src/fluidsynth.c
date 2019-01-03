/*
*  Copyright (C) 1994-1995  Apogee Software, Ltd.
*  Copyright (C) 2002-2015  icculus.org, GNU/Linux port
*  Copyright (C) 2017-2018  Steven LeVesque
*/

/*
    Reimplementation of ROTT's music routines, using fluidsynth.
*/

#include "music.h"
#include "fluidsynth.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

static int isFluidInit = 0;

static int fadeActive = 0;

static fluid_settings_t * fsettings = NULL;
static fluid_synth_t * fsynth = NULL;
static fluid_audio_driver_t * faudiodriver = NULL;
static fluid_player_t * fplayer = NULL;

char *MUSIC_ErrorString( int ErrorNumber ){}


int   MUSIC_Init( int SoundCard, int Address )
{
    if (isFluidInit)
    {
        Error("Fluidsynth is already initialized! \n");
        return(MUSIC_Error);
    }
    
    fsettings = new_fluid_settings();
    
    //TODO: Change audio driver depending on what OS the user is using
    //ex: dsound for windows, pulse audio for Linux users...
    //Is there a way to retrieve a list of audio drivers fluidsynth can work with???
    
    if (__WIN32__)
        fluid_settings_setstr(fsettings, "audio.driver", "dsound");
    else
        fluid_settings_setstr(fsettings, "audio.driver", "pulseaudio");
    
    
    
    fsynth = new_fluid_synth(fsettings);
    
    faudiodriver = new_fluid_audio_driver(fsettings, fsynth);
    fplayer = new_fluid_player(fsynth);
    
    //TODO: Allow users to load their own soundfonts
    //Right now, we're using a soundfont based on the Roland SC-55
    fluid_synth_sfload(fsynth, "Scc1t2.sf2", 1);
    fluid_settings_setint(fsettings, "player.reset-synth", 0);
    
    isFluidInit = 1;
    
    return MUSIC_Ok;

}

int   MUSIC_Shutdown( void )
{
    if (isFluidInit)
    {
        delete_fluid_player(fplayer);
        delete_fluid_audio_driver(faudiodriver);
        delete_fluid_synth(fsynth);
        delete_fluid_settings(fsettings);
        isFluidInit = 0;
    }

    return MUSIC_Ok;
}

int songsize;
unsigned int songLength;

int   MUSIC_PlaySong( unsigned char *song, int loopflag )
{
    MUSIC_StopSong();
    
    int memloadSuccess = fluid_player_add_mem(fplayer, song, songsize); 
    
    if(memloadSuccess == FLUID_FAILED)
    {
        Error("FLUID_FAILED: Attempt to load MIDI track from memory into fluidsynth player failed! \n");
        return MUSIC_Error;
    }
    
    if (loopflag == MUSIC_LoopSong)
    {
        loopflag = -1;//-1 means loop indefinetly in fluidsynth
    }
    else
    {
        loopflag = 1; //just play once
    }
    
    fluid_player_set_loop(fplayer, loopflag); 
    
    songLength = fluid_player_get_total_ticks(fplayer);
    
    int playSuccess = fluid_player_play(fplayer);
    
    if (playSuccess == 0)
        return MUSIC_Ok;
    else
        return MUSIC_Error;

}

int   MUSIC_PlaySongROTT(unsigned char *song, int size, int loopflag)
{
    MUSIC_StopSong();

    int memloadSuccess = fluid_player_add_mem(fplayer, song, size);
    
    songsize = size;
    
    if(memloadSuccess == FLUID_FAILED)
    {
        Error("FLUID_FAILED: Attempt to load MIDI track from memory into fluidsynth player failed! \n");
        return MUSIC_Error;
    }
    
    fluid_player_set_loop(fplayer, loopflag == MUSIC_LoopSong ? -1 : 1); 
    
    int playSuccess = fluid_player_play(fplayer);
    
    if (playSuccess == 0)
        return MUSIC_Ok;
    else
        return MUSIC_Error;

}


void  MUSIC_SetVolume( int volume ) 
{
    double val = ((double)volume) * 0.00390625; //(1/256)
    fluid_settings_setnum(fsettings, "synth.gain", val);
}

int   MUSIC_GetVolume( void ) 
{
    double val; 
    fluid_settings_getnum(fsettings, "synth.gain", &val);
    return (int) (val / 0.00390625);
}

void  MUSIC_SetLoopFlag( int loopflag ) 
{
    fluid_player_set_loop(fplayer, loopflag == MUSIC_LoopSong ? -1 : 1);
}

int   MUSIC_SongPlaying( void ) 
{
    return (fluid_player_get_status(fplayer) == FLUID_PLAYER_PLAYING ? 1 : 0 );
}

int   MUSIC_StopSong( void )
{
    fluid_player_stop(fplayer);
    MUSIC_Shutdown();
    MUSIC_Init(0, 0);
    return MUSIC_Ok;
}

int   MUSIC_FadeVolume( int tovolume, int milliseconds ) 
{
    double fadeRate;
    double curVol;
    double toVol;
    
    fluid_settings_getnum(fsettings, "synth.gain", &curVol);
    
    toVol = ((double)tovolume) * 0.00390625;
    fadeRate = (double)abs(((curVol - tovolume)/milliseconds));
        
    fadeActive = 1;
    
    toVol = ((double)tovolume) * 0.00390625;
    while(curVol > toVol)
    {
        if (toVol > curVol)
            curVol+=fadeRate;
        else if (curVol < toVol)
            curVol-=fadeRate;
        
        
        fluid_settings_setnum(fsettings, "synth.gain", curVol);
        
    }
    
    fadeActive = 0;
    
    return MUSIC_Ok;
}

// ROTT Special - SBF
//int   MUSIC_PlaySongROTT(unsigned char *song, int size, int loopflag);

//Everything below this comment may or may not be implemented in a future update

void  MUSIC_Continue( void ) 
{
    //Error("Not implmented yet");
}

void  MUSIC_Pause( void ) 
{
    //Error("Not implmented yet");
}

void  MUSIC_SetSongTick( unsigned long PositionInTicks ) 
{
    //Error("%s Not implmented yet", __func__);
}

void  MUSIC_SetSongTime( unsigned long milliseconds ) 
{

}
void  MUSIC_SetSongPosition( int measure, int beat, int tick ) 
{
    //Error("%s Not implmented yet", __func__);
}
void  MUSIC_GetSongPosition( songposition *pos ) 
{

}
void  MUSIC_GetSongLength( songposition *pos ) 
{

}

int   MUSIC_FadeActive( void ) 
{
    return fadeActive == 1 ? 1 : 0;
}
void  MUSIC_StopFade( void ) 
{
    //fadeActive = 0;
}
void  MUSIC_RegisterTimbreBank( unsigned char *timbres ) 
{
    //Error("Not implmented yet");
}

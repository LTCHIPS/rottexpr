/*
*  Copyright (C) 1994-1995  Apogee Software, Ltd.
*  Copyright (C) 2002-2015  icculus.org, GNU/Linux port
*  Copyright (C) 2017-2018  Steven LeVesque
*/

#include "music.h"
#include "fluidsynth.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

#include "rt_def.h"      // ROTT music hack
#include "rt_cfg.h"      // ROTT music hack
#include "rt_util.h"     // ROTT music hack

static char warningMessage[80];
static char errorMessage[80];
static FILE *debug_file = NULL;
static int initialized_debugging = 0;


static int isFluidInit = 0;

static fluid_settings_t * fsettings = NULL;
static fluid_synth_t * fsynth = NULL;
static fluid_audio_driver_t * faudiodriver = NULL;
static fluid_player_t * fplayer = NULL;


char *MUSIC_ErrorString(int ErrorNumber)
{
    switch (ErrorNumber)
    {
    case MUSIC_Warning:
        return(warningMessage);

    case MUSIC_Error:
        return(errorMessage);

    case MUSIC_Ok:
        return("OK; no error.");

    case MUSIC_ASSVersion:
        return("Incorrect sound library version.");

    case MUSIC_SoundCardError:
        return("General sound card error.");

    case MUSIC_InvalidCard:
        return("Invalid sound card.");

    case MUSIC_MidiError:
        return("MIDI error.");

    case MUSIC_MPU401Error:
        return("MPU401 error.");

    case MUSIC_TaskManError:
        return("Task Manager error.");

    case MUSIC_FMNotDetected:
        return("FM not detected error.");

    case MUSIC_DPMI_Error:
        return("DPMI error.");

    default:
        return("Unknown error.");
    } // switch

    assert(0);    // shouldn't hit this point.
    return(NULL);
} // MUSIC_ErrorString


int   MUSIC_Init( int SoundCard, int Address )
{
    if (isFluidInit)
    {
        Error("Fluidsynth is already initialized! \n");
        return(MUSIC_Error);
    
    }
    fsettings = new_fluid_settings();
    fluid_settings_setstr(fsettings, "audio.driver", "alsa");
   
    //fluid_settings_setint(fsettings, "synth.cpu-cores", 2);
    
    //fluid_settings_setstr(fsettings, "synth.verbose", 1);
    
    fsynth = new_fluid_synth(fsettings);
    
    faudiodriver = new_fluid_audio_driver(fsettings, fsynth);
    fplayer = new_fluid_player(fsynth);
    
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


int   MUSIC_PlaySong( unsigned char *song, int loopflag )
{
    MUSIC_StopSong();
    
/*
    GetPathFromEnvironment(filename, ApogeePath, "tmpsong.mid");
    handle = SafeOpenWrite(filename);

    SafeWrite(handle, song, size);
    close(handle);
*/
    int memloadSuccess = fluid_player_add_mem(fplayer, song, songsize);
    //printf("%d \n",fluid_player_add(fplayer, "tmpsong.mid")); 
    
    if(memloadSuccess == FLUID_FAILED)
    {
        Error("FLUID_FAILED: Attempt to load MIDI track from memory into fluidsynth player failed! \n");
        return MUSIC_Error;
    }
    
    //fluid_player_add(fplayer, "tmpsong.mid");
    
    if (loopflag == MUSIC_LoopSong)
    {
        loopflag = -1;//-1 means loop indefinetly in fluidsynth
    }
    else
    {
        loopflag = 1;
    }
    
    fluid_player_set_loop(fplayer, loopflag); 
    
    int playSuccess = fluid_player_play(fplayer);
    
    if (playSuccess == 0)
        return MUSIC_Ok;
    else
        return MUSIC_Error;
    //fluid_player_join(fplayer);
    
    //return MUSIC_Ok;
    


}

int   MUSIC_PlaySongROTT(unsigned char *song, int size, int loopflag)
{
    MUSIC_StopSong();
    
    
    fluid_settings_setint(fsettings, "player.reset-synth", 0);

    int memloadSuccess = fluid_player_add_mem(fplayer, song, size);
    //printf("%d \n",fluid_player_add(fplayer, "tmpsong.mid")); 
    
    songsize = size;
    
    if(memloadSuccess == FLUID_FAILED)
    {
        Error("FLUID_FAILED: Attempt to load MIDI track from memory into fluidsynth player failed! \n");
        return MUSIC_Error;
    }
    
    //fluid_player_add(fplayer, "tmpsong.mid");
    
    fluid_player_set_loop(fplayer, loopflag == MUSIC_LoopSong ? -1 : 1); 
    
    int playSuccess = fluid_player_play(fplayer);
    
    if (playSuccess == 0)
        return MUSIC_Ok;
    else
        return MUSIC_Error;
    //fluid_player_join(fplayer);
    
    //return MUSIC_Ok;


}


void  MUSIC_SetMaxFMMidiChannel( int channel ) {Error("Not implmented yet");}

void  MUSIC_SetVolume( int volume ) 
{
    double val = ((double)volume) * 0.00390625; //(1/256)
    fluid_settings_setnum(fsettings, "synth.gain", val);
}


void  MUSIC_SetMidiChannelVolume( int channel, int volume ) {Error("Not implmented yet");}
void  MUSIC_ResetMidiChannelVolumes( void ) {Error("Not implmented yet");}

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
void  MUSIC_Continue( void ) {Error("Not implmented yet");}
void  MUSIC_Pause( void ) {Error("Not implmented yet");}

int   MUSIC_StopSong( void ) 
{ 
    //fluid_player_stop(fplayer);
    MUSIC_Shutdown();
    MUSIC_Init(0, 0);
    return MUSIC_Ok;
}

// ROTT Special - SBF
//int   MUSIC_PlaySongROTT(unsigned char *song, int size, int loopflag);

void  MUSIC_SetContext( int context ){Error("%s Not implmented yet", __func__);}
int   MUSIC_GetContext( void ) {Error("%s Not implmented yet", __func__); return 0;} 
void  MUSIC_SetSongTick( unsigned long PositionInTicks ) {Error("%s Not implmented yet", __func__);}
void  MUSIC_SetSongTime( unsigned long milliseconds ) {Error("%s Not implmented yet", __func__);}
void  MUSIC_SetSongPosition( int measure, int beat, int tick ) {Error("%s Not implmented yet", __func__);}
void  MUSIC_GetSongPosition( songposition *pos ) {Error("%s Not implmented yet", __func__);}
void  MUSIC_GetSongLength( songposition *pos ) {Error("%s Not implmented yet", __func__);}
int   MUSIC_FadeVolume( int tovolume, int milliseconds ) {Error("%s Not implmented yet", __func__); return 0;}
int   MUSIC_FadeActive( void ) {Error("%s Not implmented yet", __func__); return 0;}
void  MUSIC_StopFade( void ) {Error("%s Not implmented yet", __func__);}
/*
//void  MUSIC_RerouteMidiChannel( int channel, int cdecl ( *function )( int event, int c1, int c2 ) ){}
*/
void  MUSIC_RegisterTimbreBank( unsigned char *timbres ) {Error("Not implmented yet");}

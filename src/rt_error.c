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
#include <dos.h>
#include <errno.h>
#include <io.h>
#include <stdio.h>
#include <conio.h>
#include <stdarg.h>
#include <mem.h>
#include <ctype.h>
#include "rt_def.h"
#include "rt_str.h"
#include "rt_error.h"
#include "rt_menu.h"
#include "isr.h"
#include "w_wad.h"
#include "z_zone.h"
#include "rt_vid.h"
#include "rt_util.h"
#include "modexlib.h"


//*****************************************************************************
//
//                          HARD ERROR ROUTINES
//
//****************************************************************************

#define WINDOWUX           7
#define WINDOWUY           76
#define WINDOWLX           138
#define WINDOWLY           158

#define MESSAGEBOXCOLOR    166


#define DISKERROR          0x4000         // bit 15 (of deverr)
#define IGNOREAVAILABLE    0x1000         // bit 13   (bit 14 isn't used)
#define RETRYAVAILABLE     0x800          // bit 12
#define FAILAVAILABLE      0x400          // bit 11
#define LOCATION           0x300          // bit 10 and 9
#define READWRITEERROR     0x80           // bit 8
#define DRIVEOFERROR       0x0F           // low-order byte

#define DIVISIONINT                0x00

// Globals

boolean DivisionError = false;

// Statics


static char ErrorCodes[13][25] =
{
    "Write-protected disk\0",
    "Unknown unit\0",
    "Drive not ready\0",
    "Unknown command\0",
    "CRC error in data\0",
    "Bad drive struct length\0",
    "Seek error\0",
    "Unknown media type\0",
    "Sector not found\0",
    "Printer out of paper\0",
    "Write fault\0",
    "Read fault\0",
    "General failure\0"
};

static char Drives[7][3] =
{
    "A\0",
    "B\0",
    "C\0",
    "D\0",
    "E\0",
    "F\0",
    "G\0"
};

static char Locations[4][11] =
{
    "MS-DOS\0",
    "FAT\0",
    "Directory\0",
    "Data area\0"
};

static char ReadWrite[2][6] =
{
    "Read\0",
    "Write\0"
};

static boolean ErrorHandlerStarted=false;
void (__interrupt __far *olddivisr) () = NULL;

//******************************************************************************
//
// UL_UserMessage ()
//
//******************************************************************************

void UL_UserMessage (int x, int y, char *str, ...) __attribute__((format(printf,3,4)))
{
    va_list strptr;
    char buf[128];
    int width, height;

    memset (&buf[0], 0, sizeof (buf));
    va_start (strptr, str);
    vsprintf (&buf[0], str, strptr);
    va_end (strptr);

    if ( *(byte *)0x449 == 0x13)
    {
        CurrentFont = tinyfont;

        WindowW=160;
        WindowH=100;
        WindowX=80;
        WindowY=50;

        US_MeasureStr (&width, &height, &buf[0]);

        width  += (CurrentFont->width[1] << 1);
        height += (CurrentFont->height << 1);

        VL_Bar (x, y, WindowW-2, WindowH, MESSAGEBOXCOLOR);

        PrintX = x+CurrentFont->width[1];
        PrintY = y+CurrentFont->height;

        US_CPrint (&buf[0]);

        displayofs=bufferofs;

        OUTP(CRTC_INDEX, CRTC_STARTHIGH);
        OUTP(CRTC_DATA,((displayofs&0x0000ffff)>>8));


        bufferofs += screensize;
        if (bufferofs > page3start)
            bufferofs = page1start;
    }
    else
        printf("%s\n",&buf[0]);
}

//****************************************************************************
//
// UL_GeneralError ()
//
//****************************************************************************

int UL_GeneralError (int code)
{
    boolean done = false;
    int retval = 0;

    UL_UserMessage (80, 50, "Device Error!\n%s.\n \n(A)bort  (R)etry\n",
                    ErrorCodes[code]);

    if (KeyboardStarted==true)
    {
        while (!done)
        {
            if (Keyboard[sc_A])
            {
                retval = 1;
                done = true;

                while (Keyboard[sc_A])
                    ;
            }
            else if (Keyboard[sc_R])
            {
                retval = 0;
                done = true;

                while (Keyboard[sc_R])
                    ;
            }
        }
    }
    else
    {
        while (!done)
        {
            if (kbhit())
            {
                char ch;

                ch=toupper(getch());
                if (ch=='A')
                {
                    retval = 1;
                    done = true;
                }
                else if (ch=='R')
                {
                    retval = 0;
                    done = true;
                }
            }
        }
    }


    return (retval);
}


//****************************************************************************
//
// UL_DriveError ()
//
//****************************************************************************

int UL_DriveError (int code, int location, int rwerror, int whichdrive)
{
    boolean done = false;
    int retval   = 0;

    UL_UserMessage (80, 50,
                    "Drive Error!\n%s.\nOn drive %s.\nLocation: %s.\n%s error.\n(A)bort  (R)etry\n",
                    ErrorCodes[code], Drives[whichdrive],
                    Locations[location], ReadWrite[rwerror]);

    if (KeyboardStarted==true)
    {
        while (!done)
        {
            if (Keyboard[sc_A])
            {
                retval = 1;
                done = true;

                while (Keyboard[sc_A])
                    ;
            }
            else if (Keyboard[sc_R])
            {
                retval = 0;
                done = true;

                while (Keyboard[sc_R])
                    ;
            }
        }
    }
    else
    {
        while (!done)
        {
            if (kbhit())
            {
                char ch;

                ch=toupper(getch());
                if (ch=='A')
                {
                    retval = 1;
                    done = true;
                }
                else if (ch=='R')
                {
                    retval = 0;
                    done = true;
                }
            }
        }
    }

    return (retval);
}


//****************************************************************************
//
//	UL_harderr ()
//
//****************************************************************************

int __far UL_harderr (unsigned deverr, unsigned errcode, unsigned far *devhdr)
{
    int DiskError      = 0;    // Indicates if it was a disk error
    int IgnoreAvail    = 0;    // if "ignore" response is available
    int RetryAvail     = 0;    // if "retry" response is available
    int FailAvail      = 0;    // if "fail" response is available
    byte ErrorLocation = 0;    // Location of error
    byte RWerror       = 0;    // Read/Write error (0 == read, 1 == write)
    byte whichDrive    = 0;    // Drive the error is on (0 == A, 1 == B, ...)
    int action;

    unsigned temp;
    temp = *devhdr;

    // Check errors
    DiskError     = (deverr & DISKERROR);
    IgnoreAvail   = (deverr & IGNOREAVAILABLE);
    RetryAvail    = (deverr & RETRYAVAILABLE);
    FailAvail     = (deverr & FAILAVAILABLE);
    ErrorLocation = ((deverr & LOCATION) >> 8);
    RWerror       = (deverr & READWRITEERROR);

    if (DiskError == 0)
        action = UL_GeneralError (errcode);
    else
        action = UL_DriveError (errcode, ErrorLocation, RWerror, whichDrive);

    if (action)
        Error ("USER BREAK : ROTT aborted.\n");
    return (_HARDERR_RETRY);
}


//****************************************************************************
//
//	UL_DivisionISR ()
//
//****************************************************************************

extern byte * colormap;

void __interrupt __far UL_DivisionISR ( void )
{
// acknowledge the interrupt

    SetBorderColor (*(colormap+(((100-10)>>2)<<8)+160));
    DivisionError = true;
    OUTP (0x20, 0x20);
}


//****************************************************************************
//
//	UL_ErrorStartup ()
//
//****************************************************************************

void UL_ErrorStartup ( void )
{
    if (ErrorHandlerStarted==true)
        return;
    ErrorHandlerStarted=true;
    _harderr (UL_harderr);     // Install hard error handler
    UL_StartupDivisionByZero();
}

//****************************************************************************
//
//	UL_ErrorShutdown ()
//
//****************************************************************************

void UL_ErrorShutdown ( void )
{
    if (ErrorHandlerStarted==false)
        return;
    ErrorHandlerStarted=false;
    UL_ShutdownDivisionByZero();
}


/*
===============
=
= UL_StartupDivisionByZero
=
===============
*/

void UL_StartupDivisionByZero ( void )
{
    olddivisr = _dos_getvect(DIVISIONINT);
    _dos_setvect (DIVISIONINT, UL_DivisionISR);
}

/*
===============
=
= UL_ShutdownDivisionByZero
=
===============
*/

void UL_ShutdownDivisionByZero ( void )
{
    _dos_setvect (DIVISIONINT, olddivisr);
}


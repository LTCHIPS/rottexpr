Based off of the icculus Rise of the Triad source port (can be found here: https://icculus.org/rott/)

This source port of Rise of the Triad features the following:

New gameplay options including the following
	Blitzguards being outfitted with anything from the missile arsenal!
	You can now pick up ammo from dropped missile weapons!
	A special respawning enemy game mode called ZomROTT, where you have to gib your enemies to keep them from coming back!
	Toggleable Auto Aim
	Auto Aim for Missile Weapons
	...and more

Top Bar of the status bar isn't off to the side anymore!

And all of the improvements made in the icculus source port...




To run the game you'll need the following things:

SDL.dll (https://www.libsdl.org/download-1.2.php)

SDL_mixer.dll (https://www.libsdl.org/projects/SDL_mixer/release-1.2.html)

And Either:

DARKWAR.RTC
DARKWAR.RTL
DARKWAR.WAD
DEMO1_3.DMO
DEMO2_3.DMO
DEMO3_3.DMO
DEMO4_3.DMO
REMOTE1.RTS

Or

HUNTBGIN.RTC
HUNTBGIN.RTL
HUNTBGIN.WAD
DEMO1_3.DMO
DEMO2_3.DMO
DEMO3_3.DMO
DEMO4_3.DMO
REMOTE1.RTS

Place all those files in the same directory as rott.exe.


BUILDING:

To build the project, you'll need the SDL1.2 development libraries (https://www.libsdl.org/download-1.2.php) 
as well as the SDL_mixer developement libaries (https://www.libsdl.org/projects/SDL_mixer/release-1.2.html, under Binary).

For linux users, just install the following two packages using "apt-get"

libsdl1.2-dev
libsdl-mixer1.2-dev

I used MinGW with gcc to build the project.

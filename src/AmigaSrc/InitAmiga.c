/*

Copyright (C) 1994-1995 Looking Glass Technologies, Inc.
Copyright (C) 2015-2018 Night Dive Studios, LLC.
Copyright (C) 2018-2020 Shockolate Project
Copyright (C) 2026 Giuseppe Perniola

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/
// InitAmiga.c - Initialize Amiga stuff and setup the application's globals.

//--------------------
//  Includes
//--------------------
#ifdef USE_SDL
#include <SDL/SDL.h>
#else
#include <graphics/gfx.h>
#include <graphics/gfxmacros.h>
#include <proto/graphics.h>
#endif // USE_SDL
#include "InitAmiga.h"
#include "Shock.h"
#include "ShockBitmap.h"
#include "shockolate_version.h"

extern bool fullscreenActive;

//  Globals

intptr_t *gScreenAddress = NULL;
int32_t gScreenRowbytes;
int32_t gPhysicalWidth;
int32_t gPhysicalHeight;
int32_t gLogicalWidth;
int32_t gLogicalHeight;

//  Time Manager routines and globals

uint32_t gShockTicks;
uint32_t *tmd_ticks = NULL;

#ifdef USE_SDL
Uint32 startTimeFPS;
#else
struct MsgPort *pTimerMsgPort = NULL;
struct timerequest *pTimerIOReq = NULL;
struct Library *TimerBase;
struct timeval startTime;
struct timeval startTimeFPS;

extern struct GfxBase *GfxBase;
struct Library *KeymapBase = NULL;
#endif // USE_SDL

void InitAmiga()
{
    INFO("Starting %s", SHOCKOLATE_VERSION);

#ifdef USE_SDL
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_AUDIO) < 0)
    {
        DEBUG("%s: Init SDL failed", __FUNCTION__);
    }

    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
#else
    GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", 40);
    if (!GfxBase)
    {
    	ERROR("Unable to open graphics.library v40+");

        exit(1);
    }
    /*if (GfxBase->ChunkyToPlanarPtr)
    {
        INFO("Found chunky-to-planar hardware");
    }*/

    KeymapBase = OpenLibrary("keymap.library", 37);
    if (!KeymapBase)
    {
        ERROR("Unable to open keymap.library v37+");

        exit(1);
    }
#endif

    InstallShockTimers(); // needed for the tick pointer
}

void InstallShockTimers()
{
#ifdef USE_SDL
    gShockTicks = 0;
    tmd_ticks = &gShockTicks;
#else
    pTimerMsgPort = CreateMsgPort();
    if (pTimerMsgPort)
    {
        pTimerIOReq = CreateIORequest(pTimerMsgPort, sizeof(struct MsgPort));
        if (pTimerIOReq)
        {
            if (OpenDevice(TIMERNAME, UNIT_VBLANK, (APTR)pTimerIOReq, 0) == 0)
            {
                TimerBase = (APTR)pTimerIOReq->tr_node.io_Device;

                GetSysTime(&startTime);
                GetSysTime(&startTimeFPS);

                gShockTicks = 0;
                tmd_ticks = &gShockTicks;

                return;
            }
        }
    }

	ERROR("Unable to init the timers");

	exit(1);
#endif // USE_SDL
}

void CleanupAndExit()
{
    gPlayingGame = false;

	CleanupFrameBuffers();

	CleanupScreenAndWindow();

#ifdef USE_SDL
    SDL_Quit();
#else
	if (TimerBase)
    {
		CloseDevice((APTR)pTimerIOReq);
	}
	DeleteIORequest(pTimerIOReq);
	DeleteMsgPort(pTimerMsgPort);
	TimerBase = 0;
	pTimerIOReq = 0;
	pTimerMsgPort = 0;

	if (!fullscreenActive)
    {
        ResetPalette();

        UnlockPubScreen(NULL, pMainScreen);
    }

	CloseLibrary(KeymapBase);
	CloseLibrary((struct Library *)GfxBase);
#endif
}

#ifdef USE_SDL
Uint32 GetMilliseconds()
{
    return SDL_GetTicks();
}
#else
ULONG GetMilliseconds()
{
	struct timeval endTime;

	GetSysTime(&endTime);
	SubTime(&endTime, &startTime);

	return endTime.tv_secs * 1000 + endTime.tv_micro / 1000;
}
#endif // USE_SDL

bool CanGetCurrentFPS()
{
#ifdef USE_SDL
    Uint32 endTime = GetMilliseconds();
    if (endTime - startTimeFPS >= 1000)
    {
        startTimeFPS = GetMilliseconds();

        return true;
    }
#else
    struct timeval endTime;

	GetSysTime(&endTime);
	SubTime(&endTime, &startTimeFPS);

	if (endTime.tv_secs * 1000 + endTime.tv_micro / 1000 >= 1000)
    {
        GetSysTime(&startTimeFPS);

        return true;
    }
#endif // USE_SDL

    return false;
}

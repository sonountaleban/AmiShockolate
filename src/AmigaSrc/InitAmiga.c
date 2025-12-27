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
#include "InitAmiga.h"
#include "Shock.h"
#include "ShockBitmap.h"
#include "shockolate_version.h"

//  Globals

intptr_t *gScreenAddress = NULL;
int32_t gScreenRowbytes;
int32_t gScreenWide, gScreenHigh;

//  Time Manager routines and globals

uint32_t gShockTicks;
uint32_t *tmd_ticks = NULL;

struct MsgPort *pTimerMsgPort = NULL;
struct timerequest *pTimerIOReq = NULL;
struct Library *TimerBase;
struct timeval startTime;

struct Library *KeymapBase = NULL;

void InitAmiga(void)
{
    INFO("Starting %s", SHOCKOLATE_VERSION);

    KeymapBase = OpenLibrary("keymap.library", 37);
    if (!KeymapBase)
    {
        ERROR("Unable to open keymap.library");

        exit(1);
    }

    pMainScreen = LockPubScreen("Workbench");
    if (!pMainScreen)
    {
        ERROR("Unable to lock the Workbench screen");

        exit(1);
    }

    InstallShockTimers(); // needed for the tick pointer
}

void InstallShockTimers(void)
{
    pTimerMsgPort = CreateMsgPort();
	pTimerIOReq = CreateIORequest(pTimerMsgPort, sizeof(struct MsgPort));
	if (pTimerIOReq)
    {
		if (OpenDevice(TIMERNAME, UNIT_VBLANK, (APTR)pTimerIOReq, 0) == 0)
		{
			TimerBase = (APTR)pTimerIOReq->tr_node.io_Device;

			GetSysTime(&startTime);

			gShockTicks = 0;
            tmd_ticks = &gShockTicks;

			return;
		}
	}

	ERROR("Unable to init the timers");

	exit(1);
}

void CleanupAndExit(void)
{
	CleanupScreenBitmaps();

	CleanupWindow();

	if (TimerBase)
    {
		CloseDevice((APTR)pTimerIOReq);
	}
	DeleteIORequest(pTimerIOReq);
	DeleteMsgPort(pTimerMsgPort);
	TimerBase = 0;
	pTimerIOReq = 0;
	pTimerMsgPort = 0;

	UnlockPubScreen(NULL, pMainScreen);

	CloseLibrary(KeymapBase);
}

ULONG GetMilliseconds()
{
	struct timeval endTime;

	GetSysTime(&endTime);
	SubTime(&endTime, &startTime);

	return (endTime.tv_secs * 1000 + endTime.tv_micro / 1000);
}

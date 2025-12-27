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
// InitAmiga.h - Initialize Amiga stuff and setup the application's globals.

#include <stdbool.h>
#include <time.h>
#include <proto/intuition.h>
#include <proto/gadtools.h>
#include <proto/graphics.h>
#include <proto/layers.h>
#include <proto/exec.h>
#include <proto/dos.h>
#include <proto/exec.h>
#include <proto/timer.h>
#include <intuition/intuition.h>
#include <devices/timer.h>
#include "lg_types.h"

// How many tick passed since game startup
extern uint32_t gShockTicks;

// Pointer to screen
extern intptr_t *gScreenAddress;
extern int32_t gScreenRowbytes;
// Size of current window
extern int32_t gScreenWide, gScreenHigh;

//--------------------
//  Prototypes
//--------------------

/// Initialize Amiga stuff.
void InitAmiga(void);

void CleanupAndExit(void);

/// Startup the SystemShock timer.
void InstallShockTimers(void);

ULONG GetMilliseconds();

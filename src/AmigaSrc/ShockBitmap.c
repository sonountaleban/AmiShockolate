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
// ShockBitmap.c - Manages off-screen bitmaps and palettes.

//--------------------
//  Includes
//--------------------
#include "InitAmiga.h"
#include "Shock.h"
#include "ShockBitmap.h"
#include "2d.h"
#include "shockolate_version.h"

//--------------------
//  Globals
//--------------------
UBYTE *mainScreenDrawSurface = NULL;
UBYTE *offscreenDrawSurface = NULL;
ULONG mainScreenDrawSurfaceSize = 0;
ULONG offscreenDrawSurfaceSize = 0;
struct RastPort *pMainScreenRastPort = NULL;

void ChangeScreenSize(int width, int height)
{
    int deltaWidth = width - gScreenWide;
    int deltaHeight = height - gScreenHigh;

    if (deltaWidth == 0 && deltaHeight == 0)
    {
        return;
    }

    INFO("ChangeScreenSize");

    //GP
    //SDL_RenderClear(renderer);

    extern bool fullscreenActive;
    //GP
    //SDL_SetWindowFullscreen(window, fullscreenActive ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
    //SDL_SetWindowSize(window, width, height);
    //SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    //SDL_RenderSetLogicalSize(renderer, width, height);

    //SizeWindow(pMainWindow, deltaWidth, deltaHeight);

    SetupWindowAndScreenBitmaps(width, height);
}

//------------------------------------------------------------------------------------
//		Setup the window and screen bitmaps
//------------------------------------------------------------------------------------
void SetupWindowAndScreenBitmaps(int width, int height)
{
    DEBUG("SetupWindowAndScreenBitmaps %i %i", width, height);

    CleanupWindow();

    static char window_title[128];
    sprintf(window_title, "%s", SHOCKOLATE_VERSION);

    pMainWindow = OpenWindowTags(NULL,
                                 WA_Left, 0, WA_Top, 0,
                                 WA_InnerWidth, width, WA_InnerHeight, height,
                                 WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_RAWKEY,
                                 WA_Flags, WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_ACTIVATE | WFLG_SMART_REFRESH | WFLG_REPORTMOUSE,
                                 WA_Title, window_title,
                                 WA_RMBTrap, TRUE,
                                 WA_PubScreenName, "Workbench",
                                 TAG_DONE);
    if (!pMainWindow)
    {
        ERROR("Failed to create the main window");

        exit(1);
    }

    pMainRastPort = pMainWindow->RPort;

    CleanupScreenBitmaps();

    mainScreenDrawSurfaceSize = width * height;
    mainScreenDrawSurface = AllocMem(mainScreenDrawSurfaceSize, MEMF_FAST);
    if (!mainScreenDrawSurface)
    {
        ERROR("Failed to create the main screen bitmap - insufficient fast ram");

        exit(1);
    }

    offscreenDrawSurfaceSize = width * height;
    offscreenDrawSurface = AllocMem(offscreenDrawSurfaceSize, MEMF_FAST);
    if (!offscreenDrawSurface)
    {
        ERROR("Failed to create the offscreen bitmap - insufficient fast ram");

        exit(1);
    }

    pMainScreenRastPort = CreateRastPort(width, height, NULL, FALSE);
    if (!pMainScreenRastPort)
    {
        ERROR("Failed to create the main screen rastport");

        exit(1);
    }

    gScreenWide = width;
    gScreenHigh = height;

    // Point the renderer at the screen bytes
    gScreenRowbytes = width;
    gScreenAddress = mainScreenDrawSurface;

    grd_mode_cap.vbase = gScreenAddress;
}

void CleanupWindow()
{
    if (pMainWindow)
    {
        CloseWindow(pMainWindow);
        pMainWindow = NULL;
    }
}

void CleanupScreenBitmaps()
{
    if (pMainScreenRastPort)
    {
        FreeRastPort(pMainScreenRastPort);
        pMainScreenRastPort = NULL;
    }

    if (mainScreenDrawSurface)
    {
        FreeMem(mainScreenDrawSurface, mainScreenDrawSurfaceSize);
        mainScreenDrawSurface = NULL;
    }

    if (offscreenDrawSurface)
    {
        FreeMem(offscreenDrawSurface, offscreenDrawSurfaceSize);
        offscreenDrawSurface = NULL;
    }
}

struct RastPort *CreateRastPort(int width, int height, struct BitMap *pFriendBitmap, BOOL displayable)
{
    struct RastPort *rp = AllocMem(sizeof(*rp), MEMF_ANY);

    if (rp)
    {
        int depth = pFriendBitmap ? GetBitMapAttr(pFriendBitmap, BMA_DEPTH) : 8;
        struct BitMap *bm = AllocBitMap(width, height, depth, BMF_CLEAR | BMF_INTERLEAVED | BMF_MINPLANES | (displayable ? BMF_DISPLAYABLE : 0), pFriendBitmap);

        if (bm)
        {
            InitRastPort(rp);
            rp->BitMap = bm;

            return rp;
        }

        FreeMem(rp, sizeof(struct RastPort));
    }

    return NULL;
}

void FreeRastPort(struct RastPort *rp)
{
    if (rp)
    {
        FreeBitMap(rp->BitMap);
        FreeMem(rp, sizeof(struct RastPort));
    }
}

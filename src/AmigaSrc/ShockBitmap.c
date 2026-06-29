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

extern bool fullscreenActive;

//--------------------
//  Globals
//--------------------
#ifdef USE_SDL
SDL_Surface *pPrimarySurface = NULL;
SDL_Surface *pSecondarySurface = NULL;
#else
UBYTE *pPrimaryFrameBuffer = NULL;
UBYTE *pSecondaryFrameBuffer = NULL;
ULONG primaryFrameBufferSize = 0;
ULONG secondaryFrameBufferSize = 0;
struct RastPort *pPrimaryFrameBufferRastPort = NULL;
#endif // USE_SDL

void ChangeScreenSize(int width, int height)
{
    int deltaWidth = width - gLogicalWidth;
    int deltaHeight = height - gLogicalHeight;

    if (deltaWidth == 0 && deltaHeight == 0)
    {
        return;
    }

    INFO("ChangeScreenSize");

    SetupWindowScreenBitmaps(width, height);
}

//------------------------------------------------------------------------------------
//		Setup the window, screen and bitmaps
//------------------------------------------------------------------------------------
void SetupWindowScreenBitmaps(int width, int height)
{
    DEBUG("SetupWindowAndScreenBitmaps %i %i", width, height);

    CleanupScreenAndWindow();

    static char windowTitle[128];
    sprintf(windowTitle, "%s", SHOCKOLATE_VERSION);

    if (fullscreenActive)
    {
#ifdef USE_SDL
        pMainScreen = SDL_SetVideoMode(width, height, 0, SDL_SWSURFACE | SDL_HWPALETTE | SDL_DOUBLEBUF | SDL_FULLSCREEN);
        if (!pMainScreen)
        {
            ERROR("Failed to create the main screen");

            exit(1);
        }

        gPhysicalWidth = pMainScreen->w;
        gPhysicalHeight = pMainScreen->h;
#else
        pMainScreen = OpenScreenTags(NULL,
                                     SA_Left, 0, SA_Top, 0, SA_Width, width, SA_Height, height, SA_Depth, 8,
                                     SA_Type, CUSTOMSCREEN,
                                     SA_ShowTitle, FALSE,
                                     SA_Quiet, TRUE,
                                     SA_Exclusive, TRUE,
                                     SA_SysFont, 1,
                                     TAG_DONE);
        if (!pMainScreen)
        {
            ERROR("Failed to create the main screen");

            exit(1);
        }

        gPhysicalWidth = pMainScreen->Width;
        gPhysicalHeight = pMainScreen->Height;

        pMainWindow = OpenWindowTags(NULL,
                                     WA_Left, 0, WA_Top, 0, WA_Width, gPhysicalWidth, WA_Height, gPhysicalHeight,
                                     WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_RAWKEY,
                                     WA_Flags, WFLG_ACTIVATE | WFLG_SMART_REFRESH | WFLG_REPORTMOUSE | WFLG_BORDERLESS,
                                     WA_RMBTrap, TRUE,
                                     WA_CustomScreen, pMainScreen,
                                     TAG_DONE);
#endif // USE_SDL

        gLogicalWidth = width;
        gLogicalHeight = height;
    }
    else
    {
#ifdef USE_SDL
        pMainScreen = SDL_SetVideoMode(width, height, 0, SDL_SWSURFACE | SDL_ANYFORMAT);
        if (!pMainScreen)
        {
            ERROR("Failed to create the main screen");

            exit(1);
        }

        SDL_WM_SetCaption(windowTitle, NULL);

        gLogicalWidth = gPhysicalWidth = pMainScreen->w;
        gLogicalHeight = gPhysicalHeight = pMainScreen->h;
#else
        pMainScreen = LockPubScreen("Workbench");
        if (!pMainScreen)
        {
            ERROR("Unable to lock the Workbench screen");

            exit(1);
        }

        SavePalette();

        pMainWindow = OpenWindowTags(NULL,
                                     WA_Left, 0, WA_Top, 0, WA_InnerWidth, width, WA_InnerHeight, height,
                                     WA_IDCMP, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_RAWKEY,
                                     WA_Flags, WFLG_DRAGBAR | WFLG_DEPTHGADGET | WFLG_CLOSEGADGET | WFLG_ACTIVATE | WFLG_SMART_REFRESH | WFLG_REPORTMOUSE,
                                     WA_Title, windowTitle,
                                     WA_RMBTrap, TRUE,
                                     WA_PubScreenName, "Workbench",
                                     TAG_DONE);

        gPhysicalWidth = gLogicalWidth = width;
        gPhysicalHeight = gLogicalHeight = height;
#endif // USE_SDL
    }

#ifndef USE_SDL
    if (!pMainWindow)
    {
        ERROR("Failed to create the main window");

        exit(1);
    }

    pMainWindowRastPort = pMainWindow->RPort;
#endif // USE_SDL

    CleanupFrameBuffers();

#ifdef USE_SDL
    pPrimarySurface = SDL_CreateRGBSurface(SDL_SWSURFACE, gLogicalWidth, gLogicalHeight, 8, 0, 0, 0, 0);
    if (!pPrimarySurface)
    {
        ERROR("Failed to create the SDL primary surface");

        exit(1);
    }

    pSecondarySurface = SDL_CreateRGBSurface(SDL_SWSURFACE, gLogicalWidth, gLogicalHeight, 8, 0, 0, 0, 0);
    if (!pSecondarySurface)
    {
        ERROR("Failed to create the SDL secondary surface");

        exit(1);
    }

    gScreenRowbytes = pPrimarySurface->pitch;
    gScreenAddress = pPrimarySurface->pixels;
#else
    primaryFrameBufferSize = gLogicalWidth * gLogicalHeight;
    pPrimaryFrameBuffer = AllocMem(primaryFrameBufferSize, MEMF_ANY);
    if (!pPrimaryFrameBuffer)
    {
        ERROR("Failed to create the primary frame buffer - insufficient memory");

        exit(1);
    }

    secondaryFrameBufferSize = primaryFrameBufferSize;
    pSecondaryFrameBuffer = AllocMem(secondaryFrameBufferSize, MEMF_ANY);
    if (!pSecondaryFrameBuffer)
    {
        ERROR("Failed to create the secondary frame buffer - insufficient memory");

        exit(1);
    }

    pPrimaryFrameBufferRastPort = CreateRastPort(gLogicalWidth, gLogicalHeight, pMainWindowRastPort->BitMap, FALSE);
    if (!pPrimaryFrameBufferRastPort)
    {
        ERROR("Failed to create the primary frame buffer rastport");

        exit(1);
    }

    // Point the renderer at the screen bytes
    gScreenRowbytes = gLogicalWidth;
    gScreenAddress = pPrimaryFrameBuffer;
#endif // USE_SDL

    grd_mode_cap.vbase = gScreenAddress;
}

void CleanupScreenAndWindow()
{
#ifdef USE_SDL
    pMainScreen = NULL;
#else
    if (pMainWindow)
    {
        CloseWindow(pMainWindow);
        pMainWindow = NULL;
    }

    if (fullscreenActive)
    {
        CloseScreen(pMainScreen);
        pMainScreen = NULL;
    }
#endif // USE_SDL
}

void CleanupFrameBuffers()
{
#ifdef USE_SDL
    if (pPrimarySurface)
    {
        SDL_FreeSurface(pPrimarySurface);
        pPrimarySurface = NULL;
    }

    if (pSecondarySurface)
    {
        SDL_FreeSurface(pSecondarySurface);
        pSecondarySurface = NULL;
    }
#else
    if (pPrimaryFrameBufferRastPort)
    {
        FreeRastPort(pPrimaryFrameBufferRastPort);
        pPrimaryFrameBufferRastPort = NULL;
    }

    if (pPrimaryFrameBuffer)
    {
        FreeMem(pPrimaryFrameBuffer, primaryFrameBufferSize);
        pPrimaryFrameBuffer = NULL;
    }

    if (pSecondaryFrameBuffer)
    {
        FreeMem(pSecondaryFrameBuffer, secondaryFrameBufferSize);
        pSecondaryFrameBuffer = NULL;
    }
#endif // USE_SDL
}

#ifndef USE_SDL
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
#endif

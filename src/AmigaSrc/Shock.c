/*

Copyright (C) 2015-2018 Night Dive Studios, LLC.
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
//====================================================================================
//
//		System Shock - ©1994-1995 Looking Glass Technologies, Inc.
//
//		Shock.c	-	Amiga-specific initialization and main event loop.
//
//====================================================================================

//--------------------
//  Includes
//--------------------
#include <math.h>

#include "InitAmiga.h"
#include "Modding.h"
#include "OpenGL.h"
#include "Prefs.h"
#include "Shock.h"
#include "ShockBitmap.h"

#include "amaploop.h"
#include "gr2ss.h"
#include "hkeyfunc.h"
#include "mainloop.h"
#include "setup.h"
#include "shockolate_version.h"
#include "status.h"
#include "version.h"

#ifdef USE_SDL
#include "mouse.h"
#endif // USE_SDL

//--------------------
//  Globals
//--------------------
bool gPlayingGame;

grs_screen *cit_screen;
#ifdef USE_SDL
struct SDL_Surface *pMainScreen = NULL;
#else
struct Screen *pMainScreen = NULL;
struct Window *pMainWindow = NULL;
struct RastPort *pMainWindowRastPort = NULL;
#endif // USE_SDL

#ifndef __AROS__
SDL_AudioDeviceID device;
#endif

#ifndef USE_SDL
bool showFPS;
bool dontWaitTOF;
#endif
int num_args;
int frames = 0;
int fps = 0;
float averageFPS = 0.0f;
int maxFPS = 0;
char **arg_values;

extern grs_screen *svga_screen;
extern frc *svga_render_context;
extern bool fullscreenActive;

//--------------------
//  Prototypes
//--------------------
extern void init_all(void);
extern void inv_change_fullscreen(uchar on);
extern void object_data_flush(void);
extern errtype load_da_palette(void);

void ShowRecapFPS();

// see Prefs.c
extern void CreateDefaultKeybindsFile(void);
extern void LoadHotkeyKeybinds(void);
extern void LoadMoveKeybinds(void);

//------------------------------------------------------------------------------------
//		Main function.
//------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    // Save the arguments for later
    num_args = argc;
    arg_values = argv;

    // FIXME externalize this
    log_set_quiet(0);
#ifdef _DEBUG
    log_set_level(LOG_DEBUG);
#else
    log_set_level(LOG_INFO);
#endif

    INFO("Logger initialized");

    // init Amiga stuff
    atexit(CleanupAndExit);
    atexit(ShowRecapFPS);

    InitAmiga();

    // Initialize the preferences file.
    SetDefaultPrefs();
    LoadPrefs();

    // see Prefs.c
    CreateDefaultKeybindsFile(); // only if it doesn't already exist
    // even if keybinds file still doesn't exist, defaults will be set here
    LoadHotkeyKeybinds();
    LoadMoveKeybinds();

    // Process some startup arguments
    bool showSplash = !CheckArgument("NOSPLASH");
#ifndef USE_SDL
    showFPS = CheckArgument("SHOWFPS");
    dontWaitTOF = CheckArgument("DONTWAITTOF");
#endif

    // CC: Modding support! This is so exciting.
    ProcessModArgs(argc, argv);

    // Initialize
    init_all();
    setup_init();

    gPlayingGame = true;

    load_da_palette();
    gr_clear(0xFF);

    // Draw the splash screen
    INFO("Showing splash screen");
    splash_draw(showSplash);

    // Start in the Main Menu loop
    _new_mode = _current_loop = SETUP_LOOP;
    loopmode_enter(SETUP_LOOP);

    // Start the main loop
    INFO("Showing main menu, starting game loop");
    mainloop(argc, argv);

    status_bio_end();
    stop_music();

    return 0;
}

bool CheckArgument(char *arg) {
    if (arg == NULL)
        return false;

    for (int i = 1; i < num_args; i++) {
        if (strcmp(arg_values[i], arg) == 0) {
            return true;
        }
    }

    return false;
}

void InitScreen()
{
    gr_init();

    extern short svga_mode_data[];
    gr_set_mode(svga_mode_data[gShockPrefs.doVideoMode], TRUE);

    INFO("Setting up screen and render contexts");

    // Create the window, the screen and the bitmaps
    SetupWindowScreenBitmaps(grd_cap->w, grd_cap->h);

    // Setup the screen
    svga_screen = cit_screen = gr_alloc_screen(grd_cap->w, grd_cap->h);
    gr_set_screen(svga_screen);

    gr_alloc_ipal();

#ifdef USE_SDL
    SDL_ShowCursor(SDL_DISABLE);
#else
    ShowCursor(FALSE);
#endif // USE_SDL

    ScreenDraw();
}

#ifndef USE_SDL
long GUN8TO32(long col)
{
	col = col | (col << 8);
	col = col | (col << 16);

	return col;
}
#endif

#ifdef USE_SDL
SDL_Color gamePalette[256];
#else
AmigaColour gamePalette[256];
#endif // USE_SDL
bool UseCutscenePalette = FALSE; // see cutsloop.c
void SetPalette(int index, int count, uchar *pal) {
    static bool gammalut_init = 0;
    static uchar gammalut[100 - 10 + 1][256];
    if (!gammalut_init) {
        double factor = (use_opengl() ? 1.0 : 2.2); // OpenGL uses 2.2
        int i, j;
        for (i = 10; i <= 100; i++) {
            double gamma = (double)i * 1.0 / 100;
            gamma = 1 - gamma;
            gamma *= gamma;
            gamma = 1 - gamma;
            gamma = 1 / (gamma * factor);
            for (j = 0; j < 256; j++)
                gammalut[i - 10][j] = (uchar)(pow((double)j / 255, gamma) * 255);
        }
        gammalut_init = 1;
        INFO("Gamma LUT init\'ed");
    }

    int gam = gShockPrefs.doGamma;
    if (gam < 10)
        gam = 10;
    if (gam > 100)
        gam = 100;
    gam -= 10;

    for (int i = index; i < index + count; i++)
    {
        gamePalette[i].r = gammalut[gam][*pal++];
        gamePalette[i].g = gammalut[gam][*pal++];
        gamePalette[i].b = gammalut[gam][*pal++];
#ifndef __AROS__
        gamePalette[i].a = 0xff;
#endif
    }

    if (!UseCutscenePalette) {
        // Hack black!
        gamePalette[255].r = 0x0;
        gamePalette[255].g = 0x0;
        gamePalette[255].b = 0x0;
#ifndef __AROS__
        gamePalette[255].a = 0xff;
#endif
    }

#ifdef USE_SDL
    //SDL_SetColors(pMainScreen, gamePalette, 0, 256);
    SDL_SetColors(pPrimarySurface, gamePalette, 0, 256);
    SDL_SetColors(pSecondarySurface, gamePalette, 0, 256);
#else
    for (int i = 0; i < 256; i ++)
    {
        SetRGB32(&(pMainScreen->ViewPort), i, GUN8TO32(gamePalette[i].r), GUN8TO32(gamePalette[i].g), GUN8TO32(gamePalette[i].b));
    }
#endif // USE_SDL
}

#ifndef USE_SDL
ULONG savedPalette[256 * 3];

void SavePalette()
{
    GetRGB32(pMainScreen->ViewPort.ColorMap, 0, 256, savedPalette);
}

void ResetPalette()
{
    for (int i = 0; i < 256; i ++)
    {
        SetRGB32(&(pMainScreen->ViewPort), i, savedPalette[i * 3], savedPalette[i * 3 + 1], savedPalette[i * 3 + 2]);
    }
}
#endif

void ScreenDraw()
{
#ifdef USE_SDL
    SDL_BlitSurface(pPrimarySurface, NULL, pMainScreen, NULL);
#else
    // On AROS this pointer doesn't exist
    if (true/*GfxBase->ChunkyToPlanarPtr*/)
    {
        WriteChunkyPixels(pPrimaryFrameBufferRastPort, 0, 0, gLogicalWidth, gLogicalHeight, pPrimaryFrameBuffer, gScreenRowbytes);
    }
    else
    {
        // A ridiculous chunky to planar routine
        /*UBYTE colours[8];       // 8 = number of pixels to process at each cycle
        UBYTE *pBitmapSrc = pPrimaryFrameBuffer;
        int indexDst = 0;
        for (int indexSrc = 0; indexSrc < gWindowWidth * gWindowHeight; indexSrc += 8, indexDst ++, pBitmapSrc += 8)
        {
            *((ULONG *)colours) = *((ULONG *)pBitmapSrc);
            *((ULONG *)colours + 1) = *((ULONG *)pBitmapSrc + 1);

            // Fill all the 8 planes (2^8 = 256 colours)
            for (int j = 0; j < 8; j ++)
            {
                UBYTE planes = 0;
                for (int l = 0; l < 8; l ++)
                {
                    planes |= ((colours[l] >> j) & 0x1) << (7 - l);
                }

                *((UBYTE *)pPrimaryFrameBufferRastPort->BitMap->Planes[j] + indexDst) = planes;
            }
        }*/
    }

    if (fullscreenActive)
    {
        if (gPhysicalWidth == gLogicalWidth && gPhysicalHeight == gLogicalHeight)
        {
            BltBitMapRastPort(pPrimaryFrameBufferRastPort->BitMap, 0, 0, pMainWindowRastPort, 0, 0, gLogicalWidth, gLogicalHeight, 0xc0);
        }
        else
        {
            struct BitScaleArgs bms_args;

            bms_args.bsa_SrcBitMap = pPrimaryFrameBufferRastPort->BitMap;
            bms_args.bsa_DestBitMap = pMainWindowRastPort->BitMap;

            bms_args.bsa_Flags = 0;

            bms_args.bsa_SrcWidth = gLogicalWidth;
            bms_args.bsa_SrcHeight = gLogicalHeight;

            bms_args.bsa_SrcX = 0;
            bms_args.bsa_SrcY = 0;
            bms_args.bsa_DestX = 0;
            bms_args.bsa_DestY = 0;
            bms_args.bsa_XSrcFactor = gLogicalWidth;
            bms_args.bsa_XDestFactor = gPhysicalWidth;
            bms_args.bsa_YSrcFactor = gLogicalHeight;
            bms_args.bsa_YDestFactor = gPhysicalHeight;

            BitMapScale(&bms_args);
        }
    }
    else
    {
        BltBitMapRastPort(pPrimaryFrameBufferRastPort->BitMap, 0, 0, pMainWindowRastPort, pMainWindow->BorderLeft, pMainWindow->BorderTop, gLogicalWidth, gLogicalHeight, 0xc0);
    }
#endif // USE_SDL

#ifdef USE_SDL
    frames ++;

    if (CanGetCurrentFPS())
    {
        fps = frames;
        frames = 0;

        if (fps > maxFPS)
        {
            maxFPS = fps;
        }

        averageFPS = (averageFPS + fps) / 2.0f;
    }
#else
    char fpsText[16];
    if (showFPS)
    {
        frames ++;

        if (CanGetCurrentFPS())
        {
            fps = frames;
            frames = 0;

            if (fps > maxFPS)
            {
                maxFPS = fps;
            }

            averageFPS = (averageFPS + fps) / 2.0f;
        }

        sprintf(fpsText, "%d FPS", fps);

        SetAPen(pMainWindowRastPort, 2);
        Move(pMainWindowRastPort, pMainWindow->BorderLeft + pMainWindowRastPort->TxWidth, pMainWindow->BorderTop + pMainWindowRastPort->TxHeight);
        Text(pMainWindowRastPort, fpsText, strlen(fpsText));
    }
#endif // USE_SDL

#ifdef USE_SDL
    /*if (dontWaitTOF)
    {
        SDL_UpdateRect(pMainScreen, 0, 0, 0, 0);            // there needs to remove SDL_DOUBLEBUF flag
    }
    else*/
    {
        SDL_Flip(pMainScreen);
    }
#else
    if (!dontWaitTOF)
    {
        WaitTOF();
    }
#endif // USE_SDL
}

void ShowRecapFPS()
{
#ifdef USE_SDL
    printf("Max FPS: %d, average FPS: %f\n\n", maxFPS, averageFPS);
#else
    if (showFPS)
    {
        printf("Max FPS: %d, average FPS: %f\n\n", maxFPS, averageFPS);
    }
#endif // USE_SDL
}

extern int mlook_enabled;

bool MouseCaptured = FALSE;

// GP: On AROS better don't use the relative mouse: while on narive AROS it works (at least using SDL 1.2),
// on hosted AROS and AXRT it doesn't
#ifdef __AROS__
#ifdef USE_SDL
void CaptureMouse(bool capture)
{
    MouseCaptured = (capture && gShockPrefs.goCaptureMouse);

    if (!MouseCaptured && mlook_enabled && SDL_GetRelativeMouseMode())
    {
        SDL_SetRelativeMouseMode(FALSE);

        /*Uint16 centerX = (Uint16)(pMainScreen->w / 2);
        Uint16 centerY = (Uint16)(pMainScreen->h / 2);
        SDL_WarpMouse(centerX, centerY);*/
    }
    else
    {
        //SDL_SetRelativeMouseMode(MouseCaptured ? SDL_TRUE : SDL_FALSE);
        SDL_SetRelativeMouseMode(FALSE);
    }
}
#else
bool RelativeMouseModeEnabled = FALSE;
bool PointerEnabled = TRUE;

bool GetRelativeMouseMode()
{
    return RelativeMouseModeEnabled;
}

void SetRelativeMouseMode(bool set)
{
    //RelativeMouseModeEnabled = set;
}

bool QueryShowCursor()
{
    return PointerEnabled;
}

void ShowCursor(bool show)
{
    PointerEnabled = show;
}

void CaptureMouse(bool capture)
{
    MouseCaptured = (capture && gShockPrefs.goCaptureMouse);

    if (!MouseCaptured && mlook_enabled && /*SDL_GetRelativeMouseMode() == SDL_TRUE*/GetRelativeMouseMode())
    {
        SetRelativeMouseMode(FALSE);
    }
    else
    {
        SetRelativeMouseMode(MouseCaptured);
    }
}
#endif
#endif

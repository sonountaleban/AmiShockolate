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

//--------------------
//  Globals
//--------------------
bool gPlayingGame;

grs_screen *cit_screen;
struct Screen *pMainScreen = NULL;
struct Window *pMainWindow = NULL;
struct RastPort *pMainWindowRastPort = NULL;

//GP
//SDL_AudioDeviceID device;

bool showFPS;
bool dontWaitTOF;
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
int main(int argc, char **argv) {
    // Save the arguments for later

    num_args = argc;
    arg_values = argv;

    // FIXME externalize this
    log_set_quiet(0);
    log_set_level(LOG_INFO);

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
    showFPS = CheckArgument("SHOWFPS");
    dontWaitTOF = CheckArgument("DONTWAITTOF");

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

    //GP
    /*SDL_ShowCursor(SDL_DISABLE);

    SDL_RaiseWindow(window);

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_PRESENTVSYNC);
    SDL_RenderSetLogicalSize(renderer, grd_cap->w, grd_cap->h);*/
    ShowCursor(FALSE);

    // Startup OpenGL
    //GP
    //init_opengl();

    ScreenDraw();

    //GP
    //SDL_ShowWindow(window);
}

long GUN8TO32(long col)
{
	col = col | (col << 8);
	col = col | (col << 16);

	return col;
}

AmigaColour gamePalette[256];
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

    for (int i = index; i < index + count; i++) {
        gamePalette[i].r = gammalut[gam][*pal++];
        gamePalette[i].g = gammalut[gam][*pal++];
        gamePalette[i].b = gammalut[gam][*pal++];
        gamePalette[i].a = 0xff;
    }

    if (!UseCutscenePalette) {
        // Hack black!
        gamePalette[255].r = 0x0;
        gamePalette[255].g = 0x0;
        gamePalette[255].b = 0x0;
        gamePalette[255].a = 0xff;
    }

    for (int i = 0; i < 256; i ++)
    {
        SetRGB32(&(pMainScreen->ViewPort), i, GUN8TO32(gamePalette[i].r), GUN8TO32(gamePalette[i].g), GUN8TO32(gamePalette[i].b));
    }

    //GP
    /*if (should_opengl_swap())
        opengl_change_palette();*/
}

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

void ScreenDraw()
{
    //GP
    /*if (should_opengl_swap()) {
        // We want the UI background to be transparent!
        sdlPalette->colors[255].a = 0x00;

        // Draw the OpenGL view
        opengl_swap_and_restore(drawSurface);

        // Set the palette back, and we are done
        sdlPalette->colors[255].a = 0xff;
        return;
    }*/
    char fpsText[16];

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

    if (!dontWaitTOF)
    {
        WaitTOF();
    }
}

void ShowRecapFPS()
{
    if (showFPS)
    {
        printf("Max FPS: %d, average FPS: %f\n\n", maxFPS, averageFPS);
    }
}

bool MouseCaptured = FALSE;
bool RelativeMouseModeEnabled = FALSE;
bool PointerEnabled = TRUE;

extern int mlook_enabled;

bool GetRelativeMouseMode()
{
    return RelativeMouseModeEnabled;
}

void SetRelativeMouseMode(bool set)
{
    // GP: at the moment no relative mouse position
    /*RelativeMouseModeEnabled = set;
    if (set)
    {
        ModifyIDCMP(pMainWindow, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_CLOSEWINDOW | IDCMP_VANILLAKEY | IDCMP_RAWKEY | IDCMP_DELTAMOVE);
    }
    else
    {
        ModifyIDCMP(pMainWindow, IDCMP_CLOSEWINDOW | IDCMP_REFRESHWINDOW | IDCMP_MOUSEBUTTONS | IDCMP_MOUSEMOVE | IDCMP_CLOSEWINDOW | IDCMP_VANILLAKEY | IDCMP_RAWKEY);
    }*/
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

    //GP
    if (!MouseCaptured && mlook_enabled && /*SDL_GetRelativeMouseMode() == SDL_TRUE*/GetRelativeMouseMode())
    {
        //GP
        /*SDL_SetRelativeMouseMode(SDL_FALSE);

        int w, h;
        SDL_GetWindowSize(window, &w, &h);
        SDL_WarpMouseInWindow(window, w / 2, h / 2);*/
        SetRelativeMouseMode(FALSE);
    }
    else
    {
        //GP
        //SDL_SetRelativeMouseMode(MouseCaptured ? SDL_TRUE : SDL_FALSE);
        SetRelativeMouseMode(MouseCaptured);
    }
}

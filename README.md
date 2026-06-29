# AmiShockolate
Amiga porting of Shockolate, the cross platform version of System Shock.

At the moment it runs only on native AROS (x86-64 version) and Linux with AxRuntime library (https://www.axrt.org/), the latter is a wrapper that emulates the AmigaOS 3.x API on Linux and Windows. All the SDL2/OpenGL stuff has been removed, replaced by standard AmigaOS calls. However now the native AROS version is based on SDL 1.2, so that audio support has been re-enabled, but digital speeches cannot be played at the same time of the music because of limits of this older SDL.

The project has been set up on Code::Blocks for Ubuntu, but it can be adapted easily for any IDE that supports gcc compiler. To build for native AROS it is necessary to install the cross-compiler environment for AROS, see https://arosnews.github.io/how-to-cross-compile-aros-hosted-wsl/.

Available debug and release executables for both supported platforms.

The AxRuntime version has two additional shell arguments:
* `SHOWFPS` - Shows the current frame rate on the top left of the screen as well as a recap in the console output once the game quit
* `DONTWAITTOF` - Allows the game to not wait the VBlank so that it can run at full speed, but internal timings may be affected

For the installation and usage, follow the instructions of Shockolate (https://github.com/Interrupt/systemshock), in particular the res folder must have the "data" and "sound" folders taken from the original CD assets.

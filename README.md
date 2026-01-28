# AmiShockolate
Amiga porting of Shockolate, the cross platform version of System Shock.

At the moment it runs on native AROS (x86-64 version) and with AxRuntime (https://www.axrt.org/), the latter is a wrapper that emulates the AmigaOS 3.x API on Linux and Windows. All the SDL2/OpenGL stuff has been removed, replaced by standard AmigaOS calls.

The project has been set up on Code::Blocks for Ubuntu, but it can be adapted easily for any IDE that supports gcc compiler. To build for native AROS it is necessary to install the cross-compiler environment for AROS, see https://arosnews.github.io/how-to-cross-compile-aros-hosted-wsl/. However there are already the debug and release executables inside the bin folder that can run on AROS.

There is no sound at the moment, but hopefully the next release will have the audio support. Then the following step should be the porting on the classic Amiga AGA in the near future.

In this version two shell arguments have been added:
* `SHOWFPS` - Shows the current frame rate on the top left of the screen as well as a recap in the console output once the game quit
* `DONTWAITTOF` - Allows the game to not wait the VBlank so that it can run at full speed, but internal timings may be affected

For the installation and usage, follow the instructions of Shockolate (https://github.com/Interrupt/systemshock), in particular the res folder must have the "data" and "sound" folders taken from the original CD assets.

# AmiShockolate
Amiga porting of Shockolate, the cross platform version of System Shock.

At the moment it runs with AxRuntime (https://www.axrt.org/), which is a wrapper that emulates the AmigaOS 3.x API on Linux and Windows. All the SDL2/OpenGL stuff has been removed, replaced by standard AmigaOS calls.

The project has been set up on Code::Blocks for Ubuntu, but it can be adapted easily for any IDE that supports gcc compiler.

Also I think it can be built and then run on Windows as well as native AROS with minimal changes.

The next release should be able to produce the executable for native AROS (x86-64 version) and hopefully have the audio support, too. Then the following step would be the porting on the classic Amiga AGA in the near future.

For the usage and installation, follow the instructions of Shockolate (https://github.com/Interrupt/systemshock), in particular the res folder should have the "data" and "sound" folders taken from the original assets.

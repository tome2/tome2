ToME is a [rogue-like](https://en.wikipedia.org/wiki/Roguelike) game.

![Screenshot](/doc/images/screenshot.png)

## Getting Started

### Prerequisites

See below for specific distribution-specific hints, if needed.

You will need to have the following libraries installed on your system
somewhere where CMake can find them:

- [jansson](http://www.digip.org/jansson/)
- [Boost](https://www.boost.org/)

Version requirements may vary somewhat, but usually you should be
aiming for having at least a **recent** version of the above libraries.

### Option 1: Running In-Place

**This is currently the recommended option**, but it means that you
don't 'install' ToME as such, you just run it from the build
directory.

To configure for your system, run

    $ cmake .
    $ make

You should now be able to run one of the executables in ./src
to run ToME. For example, you'd run

    $ ./src/tome-x11

to start ToME with the X11 frontend.

**Important:** The current working directory must be at the root of
the source tree for the above command to run -- if it isn't, then
you'll get mysterious errors about ToME not being able to find files
(at best).


### Option 2: Installing System-Wide

To configure for your system, run

    $ cmake -DSYSTEM_INSTALL:BOOL=true .
    $ make
    $ sudo make install

You can now run ToME from anywhere and it will always use the files
installed in the system-specific location.


## Compiling on Ubuntu

To compile on an Ubuntu install, you'll need at least the 

- `cmake`
- `build-essential`
- `libjansson-dev`
- `libboost-all-dev`

packages.

Each frontend requires the additional packages listed below:

- X11: `libx11-dev`
- SDL: `libsdl-image1.2-dev` `libsdl-ttf2.0-dev`
- ncurses: `libncurses5-dev`


## Compiling on OpenBSD

As of February 2010, the OpenBSD package cmake-2.4.8p2 is too old for
building ToME. You may need to compile a newer version of CMake.

If you have X11, then a bug in CMake may cause a linker error when
linking the executable. As a workaround, set the environment variable
`LDFLAGS` when running CMake. Example:

    $ env LDFLAGS=-L/usr/X11R6/lib cmake .
    $ make

The SDL frontend also requires these packages:

- `sdl-image`
- `sdl-ttf`


## Compiling on Windows using MinGW

The source **MUST** be unpacked in a directory without spaces in the
name.

To configure and compile on Windows using MinGW, use the commands

    $ cmake -G "MinGW Makefiles"
    $ mingw32-make

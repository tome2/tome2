# Building

## Prerequisites

You will need to have the following libraries installed on your system
somewhere where CMake can find them:

- [jansson](http://www.digip.org/jansson/)
- [Boost](https://www.boost.org/)

Version requirements may vary somewhat, but usually you should be
aiming for having at least a **recent** version of the above libraries.

## Using the CMake build system

There are basically two options for ToME runs once built and the step
used to configure the build needs to incorporate the choice.

### **Option 1:** Run ToME from the build directory

**This is currently the recommended option**, but it means that you
cannot "install" ToME as such, you just run it from the build
directory.

To configure for your system, run

    $ cmake .
    $ make

You should now be able to run

    $ ./src/tome

to start ToME.

**Important:** The current working directory must be at the root of
the source tree for the above command to run -- if it isn't, then
you'll get mysterious errors about ToME not being able to find files
(at best).


### **Option 2:** Run ToME from a system install location

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

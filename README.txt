Using the CMake build system
============================

There are basically two options for how to run ToME once built.


Prerequisites
=============

You will need to have the following libraries installed
on your system somewhere where CMake can find them:

   - jansson
     See http://www.digip.org/jansson/


Option #1 : Run ToME from the build directory
=============================================

Simply run the commands below.

       $ cmake .
       $ make

You should now be able to run

       $ ./src/tome

to start ToME.

This is currently the recommended option.



Option #2: Run ToME from a system install location
==================================================

Run

        $ cmake -DSYSTEM_INSTALL:BOOL=true .
        $ make
        $ sudo make install

You can now run ToME from anywhere.

You can also use DESTDIR when installing to a different location
(useful with e.g. stow or when building distribution packages).


Compiling on Ubuntu
===================

If you're having trouble compiling on an Ubuntu install you are
probably missing the

    build-essential

package. You'll also need to install the

    libjansson-dev

package.

Each frontend requires the additional packages listed below:

   X11: libx11-dev
   SDL: libsdl-image1.2-dev, libsdl-ttf2.0-dev
   ncurses: libncurses5-dev


Compiling on OpenBSD
====================

As of February 2010, the OpenBSD package cmake-2.4.8p2 is too old for
building ToME. You may need to compile a newer version of CMake.

If you have X11, then a bug in CMake may cause a linker error when
linking the 'tome' executable. As a workaround, set the environment
variable LDFLAGS=-L/usr/X11R6/lib when running CMake. Example:

       $ env LDFLAGS=-L/usr/X11R6/lib cmake .
       $ make

The SDL frontend also requires these packages: sdl-image, sdl-ttf


Compiling on Windows using MinGW
================================

(See http://www.mingw.org/)

The source MUST be unpacked in a directory without spaces in the
name.

To compile on Windows using MinGW, use the commands

       $ cmake -G "MinGW Makefiles"
       $ mingw32-make

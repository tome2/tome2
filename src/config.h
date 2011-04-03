/* File: config.h */

/* Purpose: Angband specific configuration stuff */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */


/*
 * Look through the following lines, and where a comment includes the
 * tag "OPTION:", examine the associated "#define" statements, and decide
 * whether you wish to keep, comment, or uncomment them.  You should not
 * have to modify any lines not indicated by "OPTION".
 *
 * Note: Also examine the "system" configuration file "h-config.h"
 * and the variable initialization file "variable.c".  If you change
 * anything in "variable.c", you only need to recompile that file.
 *
 * And finally, remember that the "Makefile" will specify some rather
 * important compile time options, like what visual module to use.
 */


/*
 * OPTION: See the Makefile(s), where several options may be declared.
 *
 * Some popular options include "USE_GCU" (allow use with Unix "curses"),
 * "USE_X11" (allow basic use with Unix X11) and "USE_XAW" (allow use with
 * Unix X11 plus the Athena Widget set).
 *
 * The old "USE_NCU" option has been replaced with "USE_GCU".
 *
 * Several other such options are available for non-unix machines,
 * such as "MACINTOSH", "WINDOWS".
 *
 * You may also need to specify the "system", using defines such as
 * "SOLARIS" (for Solaris), etc, see "h-config.h" for more info.
 */


/*
 * OPTION: define "SPECIAL_BSD" for using certain versions of UNIX
 * that use the 4.4BSD Lite version of Curses in "main-gcu.c"
 */
/* #define SPECIAL_BSD */


/*
 * OPTION: Use the POSIX "termios" methods in "main-gcu.c"
 */
/* #define USE_TPOSIX */

/*
 * OPTION: Use the "termio" methods in "main-gcu.c"
 */
/* #define USE_TERMIO */

/*
 * OPTION: Use the icky BSD "tchars" methods in "main-gcu.c"
 */
/* #define USE_TCHARS */


/*
 * OPTION: Use "blocking getch() calls" in "main-gcu.c".
 * Hack -- Note that this option will NOT work on many BSD machines
 * Currently used whenever available, if you get a warning about
 * "nodelay()" undefined, then make sure to undefine this.
 */
#if defined(SYS_V)
# define USE_GETCH
#endif


/*
 * OPTION: Use the "curs_set()" call in "main-gcu.c".
 * Hack -- This option will not work on most BSD machines
 */
#ifdef SYS_V
# define USE_CURS_SET
#endif


/*
 * OPTION: Include "ncurses.h" instead of "curses.h" in "main-gcu.c"
 */
/* #define USE_NCURSES */


/*
 * OPTION: for multi-user machines running the game setuid to some other
 * user (like 'games') this SAFE_SETUID option allows the program to drop
 * its privileges when saving files that allow for user specified pathnames.
 * This lets the game be installed system wide without major security
 * concerns.  There should not be any side effects on any machines.
 *
 * This will handle "gids" correctly once the permissions are set right.
 */
#define SAFE_SETUID


/*
 * This flag enables the "POSIX" methods for "SAFE_SETUID".
 */
#if defined(_POSIX_SAVED_IDS) && !(defined(SUNOS) && !defined(SOLARIS)) && !defined(__APPLE__)
# define SAFE_SETUID_POSIX
#endif


/*
 * OPTION: Allow characteres to be "auto-rolled"
 */
#define ALLOW_AUTOROLLER


/*
 * OPTION: Allow monsters to "flee" when hit hard
 */
#define ALLOW_FEAR

/*
 * OPTION: Allow monsters to "flee" from strong players
 */
#define ALLOW_TERROR


/*
 * OPTION: Handle signals
 */
#define HANDLE_SIGNALS


/*
 * Allow "Wizards" to yield "high scores"
 */
/* #define SCORE_WIZARDS */

/*
 * Allow "Borgs" to yield "high scores"
 */
/*#define SCORE_BORGS*/

/*
 * Allow "Cheaters" to yield "high scores"
 */
/* #define SCORE_CHEATERS */



/*
 * OPTION: Maximum flow depth
 */
#define MONSTER_FLOW_DEPTH 32



/*
 * OPTION: Allow use of extended spell info	-DRS-
 */
#define DRS_SHOW_SPELL_INFO

/*
 * OPTION: Allow use of the monster health bar	-DRS-
 */
#define DRS_SHOW_HEALTH_BAR



/*
 * OPTION: Allow the use of "sound" in various places.
 */
#define USE_SOUND

/*
 * OPTION: Allow the use of "graphics" in various places
 */
#define USE_GRAPHICS


/*
 * OPTION: Hack -- Macintosh stuff
 */
#ifdef MACINTOSH

/* Do not handle signals */
# undef HANDLE_SIGNALS

#endif


/*
 * OPTION: Hack -- Windows stuff
 */
#ifdef WINDOWS

/* Do not handle signals */
# undef HANDLE_SIGNALS

#endif


/*
 * OPTION: Hack -- EMX stuff
 */
#ifdef USE_EMX

/* Do not handle signals */
# undef HANDLE_SIGNALS

#endif


/*
 * OPTION: Set the "default" path to the angband "lib" directory.
 *
 * See "main.c" for usage, and note that this value is only used on
 * certain machines, primarily Unix machines.  If this value is used,
 * it will be over-ridden by the "ANGBAND_PATH" environment variable,
 * if that variable is defined and accessable.  The final slash is
 * optional, but it may eventually be required.
 *
 * Using the value "./lib/" below tells Angband that, by default,
 * the user will run "angband" from the same directory that contains
 * the "lib" directory.  This is a reasonable (but imperfect) default.
 *
 * If at all possible, you should change this value to refer to the
 * actual location of the "lib" folder, for example, "/tmp/angband/lib/"
 * or "/usr/games/lib/angband/", or "/pkg/angband/lib".
 *
 * Additional note -- if you are planning to use makefile.org, don't bother
 * setting this variable, as it is overridden by a value set near the top of
 * that file.
 */
#ifndef DEFAULT_PATH
# define DEFAULT_PATH "./lib/"
#endif


/*
 * Where to put the user's files.
 */
#if defined(MACH_O_CARBON)
#define PRIVATE_USER_PATH "~/Library/Application Support/ToME"
#define PRIVATE_USER_PATH_DATA
#define PRIVATE_USER_PATH_APEX
#define PRIVATE_USER_PATH_MODULES
#else
#define PRIVATE_USER_PATH "~/.tome"
#endif

/*
 * OPTION: For some brain-dead computers with no command line interface,
 * namely Macintosh, there has to be some way of "naming" your savefiles.
 * The current "Macintosh" hack is to make it so whenever the character
 * name changes, the savefile is renamed accordingly.  But on normal
 * machines, once you manage to "load" a savefile, it stays that way.
 * Macintosh is particularly weird because you can load savefiles that
 * are not contained in the "lib:save:" folder, and if you change the
 * player's name, it will then save the savefile elsewhere.
 */
#if defined(MACINTOSH) || defined(WINDOWS)
/* #define SAVEFILE_MUTABLE */
#endif


/*
 * OPTION: Capitalize the "user_name" (for "default" player name)
 * This option is only relevant on SET_UID machines.
 */
#define CAPITALIZE_USER_NAME



/*
 * OPTION: Person to bother if something goes wrong.
 */
#define MAINTAINER      "darkgod@t-o-m-e.net"


/*
 * OPTION: Default font (when using X11).
 */
#define DEFAULT_X11_FONT		"9x15"

/*
 * OPTION: Default fonts (when using X11)
 */
#define DEFAULT_X11_FONT_SCREEN		DEFAULT_X11_FONT
#define DEFAULT_X11_FONT_MIRROR		DEFAULT_X11_FONT
#define DEFAULT_X11_FONT_RECALL		DEFAULT_X11_FONT
#define DEFAULT_X11_FONT_CHOICE		DEFAULT_X11_FONT



/* ToME options: */

/* Should the player know his / her starting life rate? */
/* #define SHOW_LIFE_RATE */

/* Do we want different characters for different races? */
#define VARIABLE_PLAYER_GRAPH

/* Allow hordes of 'similar' monsters */
#define MONSTER_HORDES

/* Wizard mode testing options: */

/* For testing the vaults */
/* # define FORCE_V_IDX 20 */

/* Testing upkeep */
/* # define TRACK_FRIENDS */

/*
 * Using the fast autoroller can be considered as cheating
 */
#define USE_FAST_AUTOROLLER

/*
 * Forbid the use of some race/class combinations
 */
/* #define RESTRICT_COMBINATIONS */

/*
 * Enable the CTRL + L command to quit without saving
 * Only use for debugging purpose, otherwise you are a CHEATER
 */
/* #define ALLOW_QUITTING */

/*
 * Allow makefiles to override the default file mode
 */
#ifndef FILE_MODE
#define FILE_MODE 0644
#endif

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Choose the hardware, operating system, and compiler.
 * Also, choose various "system level" compilation options.
 * A lot of these definitions take effect in "h-system.h"
 *
 * Note that you may find it simpler to define some of these
 * options in the "Makefile", especially any options describing
 * what "system" is being used.
 */

/*
 * Extract the "WINDOWS" flag from the compiler
 */
#if defined(_Windows) || defined(__WINDOWS__) || \
    defined(__WIN32__) || defined(WIN32) || \
    defined(__WINNT__) || defined(__NT__)
# ifndef WINDOWS
#  define WINDOWS
# endif
#endif



/*
 * OPTION: Define "L64" if a "long" is 64-bits.  See "h-types.h".
 * The only such platform that angband is ported to is currently
 * DEC Alpha AXP running OSF/1 (OpenVMS uses 32-bit longs).
 */
#if defined(__x86_64) || defined(__x86_64__) || defined(__amd64) || defined(__ia64) || defined(__ia64__) || defined(__mips64) || defined(__ppc64__) || defined(__PPC64__) || defined(__powerpc64__) || defined(__64BIT__) || defined(__sparc64__) || defined(__LP64__)
# define L64
#endif



/*
 * OPTION: set "SET_UID" if the machine is a "multi-user" machine.
 * This option is used to verify the use of "uids" and "gids" for
 * various "Unix" calls, and of "pids" for getting a random seed,
 * and of the "umask()" call for various reasons, and to guess if
 * the "kill()" function is available, and for permission to use
 * functions to extract user names and expand "tildes" in filenames.
 * Basically, SET_UID should *only* be set for "Unix" machines.
 */
#if !defined(WINDOWS)
# define SET_UID
#endif


/*
 * Every system seems to use its own symbol as a path separator.
 * Default to the standard Unix slash, but attempt to change this
 * for various other systems.  Note that any system that uses the
 * "period" as a separator (i.e. ACORN) will have to pretend that
 * it uses the slash, and do its own mapping of period <-> slash.
 */
#undef PATH_SEP
#define PATH_SEP "/"
#if defined(WINDOWS) || defined(WINNT)
# undef PATH_SEP
# define PATH_SEP "\\"
#endif



/*
 * Basic "types".
 *
 * Note the attempt to make all basic types have 4 letters.
 * This improves readibility and standardizes the code.
 *
 * Likewise, all complex types are at least 4 letters.
 * Thus, almost every three letter word is a legal variable.
 * But beware of certain reserved words ('for' and 'if' and 'do').
 *
 * Note that the type used in structures for bit flags should be unsigned int.
 * As long as these bit flags are sequential, they will be space smart.
 *
 * Note that on some machines, apparently "signed char" is illegal.
 *
 * It must be true that char/byte takes exactly 1 byte
 * It must be true that sind/uind takes exactly 2 bytes
 * It must be true that sbig/ubig takes exactly 4 bytes
 *
 * On Sparc's, a sint takes 4 bytes (2 is legal)
 * On Sparc's, a long takes 4 bytes (8 is legal)
 * On Sparc's, a real takes 8 bytes (4 is legal)
 *
 * Note that some files have already been included by "h-include.h"
 * These include <stdio.h> and <sys/types>, which define some types
 */


/* Error codes for function return values */
/* Success = 0, Failure = -N, Problem = +N */
typedef int errr;


/* Note that unsigned values can cause math problems */
/* An unsigned byte of memory */
typedef unsigned char byte;

/* Note that a bool is smaller than a full "int" */
/* Simple True/False type */
typedef char bool_;


/* Signed/Unsigned 16 bit value */
typedef signed short s16b;
typedef unsigned short u16b;
#define FMTs16b "%hd"
#define FMTu16b "%hu"

/* Signed/Unsigned 32 bit value */
#ifdef L64	/* 64 bit longs */
typedef signed int s32b;
typedef unsigned int u32b;
#define FMTs32b "%d"
#define FMTu32b "%u"
#else
typedef signed long s32b;
typedef unsigned long u32b;
#define FMTs32b "%ld"
#define FMTu32b "%lu"
#endif



/*
 * Hack -- Define NULL
 */
#ifndef NULL
# ifdef __STDC__
#  define NULL ((void*)0)
# else
#  define NULL ((char*)0)
# endif /* __STDC__ */
#endif /* NULL */


/*
 * The constants "TRUE" and "FALSE"
 */

#undef TRUE
#define TRUE	1

#undef FALSE
#define FALSE	0




/**** Simple "Macros" ****/

/*
 * Non-typed minimum value macro
 */
#undef MIN
#define MIN(a,b)	(((a) > (b)) ? (b)  : (a))

/*
 * Non-typed maximum value macro
 */
#undef MAX
#define MAX(a,b)	(((a) < (b)) ? (b)  : (a))

/*
 * Non-typed absolute value macro
 */
#undef ABS
#define ABS(a)		(((a) < 0)   ? (-(a)) : (a))


/*
 * Note that all "index" values must be "lowercase letters", while
 * all "digits" must be "digits".  Control characters can be made
 * from any legal characters.  XXX XXX XXX
 */
#define A2I(X)	((X) - 'a')
#define I2A(X)	((X) + 'a')
#define I2C(X)  (static_cast<char>(I2A(X)))
#define D2I(X)	((X) - '0')
#define I2D(X)	((X) + '0')
#define KTRL(X)	((X) & 0x1F)
#define ESCAPE	'\033'

#ifdef __cplusplus
} // extern "C"
#endif

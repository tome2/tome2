#pragma once

#include "h-basic.h"

#include <cstdarg>

#ifdef __cplusplus
extern "C" {
#endif

/*
 * This file provides functions very similar to "sprintf()", but which
 * not only parse some additional "format sequences", but also enforce
 * bounds checking, and allow repeated "appends" to the same buffer.
 *
 * See "z-form.c" for more detailed information about the routines,
 * including a list of the legal "format sequences".
 *
 * This file makes use "z-util.c"
 */


/**** Available Functions ****/

/* Format arguments into given bounded-length buffer */
unsigned int vstrnfmt(char *buf, unsigned int max, const char *fmt, va_list vp);

/* Simple interface to "vstrnfmt()" */
unsigned int strnfmt(char *buf, unsigned int max, const char *fmt, ...);

/* Simple interface to "vformat()" */
char *format(const char *fmt, ...);

/* Vararg interface to "quit()", using "format()" */
void quit_fmt(const char *fmt, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif

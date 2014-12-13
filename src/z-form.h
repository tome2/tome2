#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "h-basic.h"

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
extern uint vstrnfmt(char *buf, uint max, cptr fmt, va_list vp);

/* Simple interface to "vstrnfmt()" */
extern uint strnfmt(char *buf, uint max, cptr fmt, ...);

/* Simple interface to "vstrnfmt()", assuming infinite length */
extern uint strfmt(char *buf, cptr fmt, ...);

/* Simple interface to "vformat()" */
extern char *format(cptr fmt, ...);

/* Vararg interface to "quit()", using "format()" */
extern void quit_fmt(cptr fmt, ...);

#ifdef __cplusplus
} /* extern "C" */
#endif

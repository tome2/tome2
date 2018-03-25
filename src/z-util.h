#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "h-basic.h"


/*
 * Extremely basic stuff, like "streq()".
 */


/* Aux functions */
extern void (*plog_aux)(const char *);
extern void (*quit_aux)(const char *);


/**** Available Functions ****/


/* Test equality, prefix, suffix */
bool_ streq(const char *s, const char *t);
bool_ prefix(const char *s, const char *t);
bool_ suffix(const char *s, const char *t);


/* Capitalize the first letter of string. Ignores whitespace at the start of string. */
void capitalize(char *s);

/* Print an error message */
void plog(const char *str);

/* Exit, with optional message */
void quit(const char *str);


#ifdef __cplusplus
} /* extern "C" */
#endif

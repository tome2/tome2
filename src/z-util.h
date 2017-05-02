#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "h-basic.h"


/*
 * Extremely basic stuff, like "streq()".
 */


/* Aux functions */
extern void (*plog_aux)(cptr);
extern void (*quit_aux)(cptr);


/**** Available Functions ****/


/* Test equality, prefix, suffix */
bool_ streq(cptr s, cptr t);
bool_ prefix(cptr s, cptr t);
bool_ suffix(cptr s, cptr t);


/* Capitalize the first letter of string. Ignores whitespace at the start of string. */
void capitalize(char *s);

/* Print an error message */
void plog(cptr str);

/* Exit, with optional message */
void quit(cptr str);


#ifdef __cplusplus
} /* extern "C" */
#endif

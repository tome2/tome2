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
extern void (*core_aux)(cptr);


/**** Available Functions ****/


/* Test equality, prefix, suffix */
extern bool_ streq(cptr s, cptr t);
extern bool_ prefix(cptr s, cptr t);
extern bool_ suffix(cptr s, cptr t);


/* Capitalize the first letter of string. Ignores whitespace at the start of string. */
extern void capitalize(char *s);

/* Print an error message */
extern void plog(cptr str);

/* Exit, with optional message */
extern void quit(cptr str);

/* Dump core, with optional message */
extern void core(cptr str);



#ifdef __cplusplus
} /* extern "C" */
#endif

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "h-basic.h"


/*
 * Extremely basic stuff, like global temp and constant variables.
 * Also, some very useful low level functions, such as "streq()".
 * All variables and functions in this file are "addressable".
 */


/**** Available variables ****/

/* Temporary Vars */
extern char char_tmp;
extern byte byte_tmp;
extern sint sint_tmp;
extern uint uint_tmp;
extern long long_tmp;
extern huge huge_tmp;
extern errr errr_tmp;

/* Temporary Pointers */
extern cptr cptr_tmp;
extern vptr vptr_tmp;


/* Constant pointers (NULL) */
extern cptr cptr_null;
extern vptr vptr_null;


/* A bizarre vptr that always points at itself */
extern vptr vptr_self;


/* A cptr to the name of the program */
extern cptr argv0;


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

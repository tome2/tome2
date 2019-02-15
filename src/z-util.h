#pragma once

#include "h-basic.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Aux functions */
extern void (*plog_aux)(const char *);
extern void (*quit_aux)(const char *);

/* Print an error message */
void plog(const char *str);

/* Exit, with optional message */
void quit(const char *str);


#ifdef __cplusplus
} /* extern "C" */
#endif

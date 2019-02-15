#pragma once

/* Aux functions */
extern void (*plog_aux)(const char *);
extern void (*quit_aux)(const char *);

/* Print an error message */
void plog(const char *str);

/* Exit, with optional message */
void quit(const char *str);

/**
 * Capitalize the first letter of string. Ignores whitespace
 * at the start of string.
 */
void capitalize(char *s);

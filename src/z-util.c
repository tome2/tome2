/* File: z-util.c */

/* Purpose: Low level utilities -BEN- */

#include "z-util.h"

#include <assert.h>


/*
 * Determine if string "t" is equal to string "t"
 */
bool_ streq(cptr a, cptr b)
{
	if ((a == NULL) && (b == NULL)) { return TRUE; }
	if (a == NULL) { return FALSE; }
	if (b == NULL) { return FALSE; }
	return (!strcmp(a, b));
}


/*
 * Determine if string "t" is a suffix of string "s"
 */
bool_ suffix(cptr s, cptr t)
{
	int tlen = strlen(t);
	int slen = strlen(s);

	/* Check for incompatible lengths */
	if (tlen > slen) return (FALSE);

	/* Compare "t" to the end of "s" */
	return (!strcmp(s + slen - tlen, t));
}


/**
 * Captialize letter
 */
void capitalize(char *s)
{
	char *p = s;
	assert(s != NULL);

	for (; *p; p++)
	{
		if (!isspace(*p))
		{
			if (islower(*p))
			{
				*p = toupper(*p);
			}
			/* Done */
			break;
		}
	}
}


/*
 * Redefinable "plog" action
 */
void (*plog_aux)(cptr) = NULL;

/*
 * Print (or log) a "warning" message (ala "perror()")
 * Note the use of the (optional) "plog_aux" hook.
 */
void plog(cptr str)
{
	/* Use the "alternative" function if possible */
	if (plog_aux) (*plog_aux)(str);

	/* Just do a labeled fprintf to stderr */
	else (void)(fprintf(stderr, "%s\n", str));
}



/*
 * Redefinable "quit" action
 */
void (*quit_aux)(cptr) = NULL;

/*
 * Exit (ala "exit()").  If 'str' is NULL, do "exit(0)".
 * If 'str' begins with "+" or "-", do "exit(atoi(str))".
 * Otherwise, plog() 'str' and exit with an error code of -1.
 * But always use 'quit_aux', if set, before anything else.
 */
void quit(cptr str)
{
	/* Attempt to use the aux function */
	if (quit_aux) (*quit_aux)(str);

	/* Success */
	if (!str) (void)(exit(0));

	/* Extract a "special error code" */
	if ((str[0] == '-') || (str[0] == '+')) (void)(exit(atoi(str)));

	/* Send the string to plog() */
	plog(str);

	/* Failure */
	(void)(exit( -1));
}

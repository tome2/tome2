#include "z-util.h"

#include <boost/algorithm/string/predicate.hpp>
#include <cassert>

using boost::algorithm::equals;

/**
 * Captialize letter
 */
void capitalize(char *s)
{
	char *p = s;
	assert(s != nullptr);

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
void (*plog_aux)(const char *) = nullptr;

/*
 * Print (or log) a "warning" message (ala "perror()")
 * Note the use of the (optional) "plog_aux" hook.
 */
void plog(const char *str)
{
	/* Use the "alternative" function if possible */
	if (plog_aux) (*plog_aux)(str);

	/* Just do a labeled fprintf to stderr */
	else (void)(fprintf(stderr, "%s\n", str));
}



/*
 * Redefinable "quit" action
 */
void (*quit_aux)(const char *) = nullptr;

/*
 * Exit (ala "exit()").  If 'str' is NULL, do "exit(0)".
 * If 'str' begins with "+" or "-", do "exit(atoi(str))".
 * Otherwise, plog() 'str' and exit with an error code of -1.
 * But always use 'quit_aux', if set, before anything else.
 */
void quit(const char *str)
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

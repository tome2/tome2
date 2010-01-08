/* File: main-dmy.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#ifdef USE_DMY

/*
 * This file helps ToME run on nothing.
 */


#include "angband.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>

/* /me pffts Solaris */
#ifndef NAME_MAX
#define	NAME_MAX	_POSIX_NAME_MAX
#endif

/*
 * Information about a term
 */
typedef struct term_data term_data;

struct term_data
{
	term t;                  /* All term info */
};

/* Max number of windows on screen */
#define MAX_TERM_DATA 4

/* Information about our windows */
static term_data data[MAX_TERM_DATA];


/*
 * Hack -- Number of initialized "term" structures
 */
static int active = 0;




/*
 * Suspend/Resume
 */
static errr Term_xtra_dummy_alive(int v)
{
	int x, y;


	/* Suspend */
	if (!v)
	{}

	/* Resume */
	else
	{}

	/* Success */
	return (0);
}



/*
 * Init the "net" system
 */
static void Term_init_dummy(term *t)
{
	term_data *td = (term_data *)(t->data);

	/* Count init's, handle first */
	if (active++ != 0) return;
}


/*
 * Nuke the "net" system
 */
static void Term_nuke_dummy(term *t)
{
	int x, y;
	term_data *td = (term_data *)(t->data);
}




/*
 * Process events (with optional wait)
 */
static errr Term_xtra_dummy_event(int v)
{
	/* Success */
	Term_keypress('\r');
	return (0);
}

/*
 * React to changes
 */
static errr Term_xtra_dummy_react(void)
{
	/* Success */
	return (0);
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_dummy(int n, int v)
{
	term_data *td = (term_data *)(Term->data);
	char buf[2];

	/* Analyze the request */
	switch (n)
	{
		/* Clear screen */
	case TERM_XTRA_CLEAR:
		return (0);

		/* Make a noise */
	case TERM_XTRA_NOISE:
		return (0);

		/* Flush the Curses buffer */
	case TERM_XTRA_FRESH:
		return (0);

		/* Suspend/Resume curses */
	case TERM_XTRA_ALIVE:
		return (Term_xtra_dummy_alive(v));

		/* Process events */
	case TERM_XTRA_EVENT:
		return (Term_xtra_dummy_event(v));

		/* Flush events */
	case TERM_XTRA_FLUSH:
		while (!Term_xtra_dummy_event(FALSE));
		return (0);

		/* Delay */
	case TERM_XTRA_DELAY:
		return (0);

		/* React to events */
	case TERM_XTRA_REACT:
		Term_xtra_dummy_react();
		return (0);

		/* Subdirectory scan */
	case TERM_XTRA_SCANSUBDIR:
		{
			DIR *directory;
			struct dirent *entry;

			scansubdir_max = 0;

			directory = opendir(scansubdir_dir);
			if (!directory)
				return 1;

			while ((entry = readdir(directory)))
			{
				char file[PATH_MAX + NAME_MAX + 2];
				struct stat filedata;

				file[PATH_MAX + NAME_MAX] = 0;
				strncpy(file, scansubdir_dir, PATH_MAX);
				strncat(file, "/", 2);
				strncat(file, entry->d_name, NAME_MAX);
				if (!stat(file, &filedata) && S_ISDIR((filedata.st_mode)))
				{
					string_free(scansubdir_result[scansubdir_max]);
					scansubdir_result[scansubdir_max] = string_make(entry->d_name);
					++scansubdir_max;
				}
			}

			closedir(directory);
			return 0;
		}
	}

	/* Unknown */
	return (1);
}


/*
 * Actually MOVE the hardware cursor
 */
static errr Term_curs_dummy(int x, int y)
{
	term_data *td = (term_data *)(Term->data);

	/* Literally move the cursor */
	// DGDGDGD

	/* Success */
	return (0);
}


/*
 * Erase a grid of space
 * Hack -- try to be "semi-efficient".
 */
static errr Term_wipe_dummy(int x, int y, int n)
{
	/* Success */
	return (0);
}

/*
 * Place some text on the screen using an attribute
 */
static errr Term_text_dummy(int x, int y, int n, byte a, cptr s)
{
	/* Success */
	return (0);
}


/*
 * Create a window for the given "term_data" argument.
 *
 * Assumes legal arguments.
 */
static errr term_data_init_dummy(term_data *td, int rows, int cols)
{
	term *t = &td->t;

	/* Initialize the term */
	term_init(t, cols, rows, 256);

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Set some hooks */
	t->init_hook = Term_init_dummy;
	t->nuke_hook = Term_nuke_dummy;

	/* Set some more hooks */
	t->text_hook = Term_text_dummy;
	t->wipe_hook = Term_wipe_dummy;
	t->curs_hook = Term_curs_dummy;
	t->xtra_hook = Term_xtra_dummy;

	/* Save the data */
	t->data = td;

	/* Activate it */
	Term_activate(t);

	/* Success */
	return (0);
}


static void hook_quit(cptr str)
{
	/* Unused */
	(void)str;
}

/*
 * Prepare "curses" for use by the file "z-term.c"
 *
 * Installs the "hook" functions defined above, and then activates
 * the main screen "term", which clears the screen and such things.
 *
 * Someone should really check the semantics of "initscr()"
 */
errr init_dummy(int argc, char **argv)
{
	int num_term = MAX_TERM_DATA, next_win = 0;

	/* Activate hooks */
	quit_aux = hook_quit;
	core_aux = hook_quit;

	/* Create a term */
	term_data_init_dummy(&data[0], 25, 80);

	/* Remember the term */
	angband_term[0] = &data[0].t;

	/* Activate the "Angband" window screen */
	Term_activate(&data[0].t);

	/* Remember the active screen */
	term_screen = &data[0].t;

	/* Success */
	return (0);
}
#endif

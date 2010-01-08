/* File: main-gcu.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


/*
 * This file helps Angband run on Unix/Curses machines.
 *
 *
 * To use this file, you must define "USE_NET" in the Makefile.
 *
 *
 * Note that this file is not "intended" to support non-Unix machines,
 * nor is it intended to support VMS or other bizarre setups.
 *
 * Also, this package assumes that the underlying "curses" handles both
 * the "nonl()" and "cbreak()" commands correctly, see the "OPTION" below.
 *
 * This code should work with most versions of "curses" or "ncurses",
 * and the "main-ncu.c" file (and USE_NCU define) are no longer used.
 *
 * See also "USE_CAP" and "main-cap.c" for code that bypasses "curses"
 * and uses the "termcap" information directly, or even bypasses the
 * "termcap" information and sends direct vt100 escape sequences.
 *
 * This file provides up to 4 term windows.
 *
 * This file will attempt to redefine the screen colors to conform to
 * standard Angband colors.  It will only do so if the terminal type
 * indicates that it can do so.  See the page:
 *
 *     http://www.umr.edu/~keldon/ang-patch/ncurses_color.html
 *
 * for information on this.
 *
 * Consider the use of "savetty()" and "resetty()".  XXX XXX XXX
 */


#include "angband.h"


#ifdef USE_NET

/* The server connection */
ip_connection net_serv_connection_forge;
ip_connection *net_serv_connection = &net_serv_connection_forge;

/* The client connection */
ip_connection net_connection_forge;
ip_connection *net_connection = &net_connection_forge;

#define PACKET_STOP             255
#define PACKET_TEXT             254
#define PACKET_CLEAR            253

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
static errr Term_xtra_net_alive(int v)
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
static void Term_init_net(term *t)
{
	term_data *td = (term_data *)(t->data);

	/* Count init's, handle first */
	if (active++ != 0) return;
}


/*
 * Nuke the "net" system
 */
static void Term_nuke_net(term *t)
{
	int x, y;
	term_data *td = (term_data *)(t->data);
}




/*
 * Process events (with optional wait)
 */
static errr Term_xtra_net_event(int v)
{
	int i, k;

	char buf[2];

	/* Wait */
	if (v)
	{
		/* Wait for one byte */
		i = zsock.read_simple(net_connection, buf, 1);
#if 1
		/* Hack -- Handle bizarre "errors" */
		//                if (!i) exit_game_panic();
		if (!i) return 1;
#else
		/* Try again(must be a new connection) */
		while (!i)
			i = zsock.read_simple(net_connection, buf, 1);
#endif
	}

	/* Do not wait */
	else
	{
		/* Read one byte, if possible */
		if (zsock.can_read(net_connection))
			zsock.read_simple(net_connection, buf, 1);
	}

	/* Ignore "invalid" keys */
	if (!buf[0]) return (1);

	/* Enqueue the keypress */
	Term_keypress(buf[0]);

	/* Success */
	return (0);
}

/*
 * React to changes
 */
static errr Term_xtra_net_react(void)
{

#ifdef A_COLOR

	int i;

	/* Cannot handle color redefinition */
	if (!can_fix_color) return (0);

	/* Set the colors */
	for (i = 0; i < 16; i++)
	{
		/* Set one color (note scaling) */
		init_color(i,
		           angband_color_table[i][1] * 1000 / 255,
		           angband_color_table[i][2] * 1000 / 255,
		           angband_color_table[i][3] * 1000 / 255);
	}

#endif

	/* Success */
	return (0);
}


/*
 * Handle a "special request"
 */
static errr Term_xtra_net(int n, int v)
{
	term_data *td = (term_data *)(Term->data);
	char buf[2];

	/* Analyze the request */
	switch (n)
	{
		/* Clear screen */
	case TERM_XTRA_CLEAR:
		buf[0] = PACKET_CLEAR;
		buf[1] = '\0';
		zsock.write_simple(net_connection, buf);
		return (0);

		/* Make a noise */
	case TERM_XTRA_NOISE:
		return (0);

		/* Flush the Curses buffer */
	case TERM_XTRA_FRESH:
		return (0);

		/* Suspend/Resume curses */
	case TERM_XTRA_ALIVE:
		return (Term_xtra_net_alive(v));

		/* Process events */
	case TERM_XTRA_EVENT:
		return (Term_xtra_net_event(v));

		/* Flush events */
	case TERM_XTRA_FLUSH:
		while (!Term_xtra_net_event(FALSE));
		return (0);

		/* Delay */
	case TERM_XTRA_DELAY:
		usleep(1000 * v);
		return (0);

		/* Get Delay of some milliseconds */
	case TERM_XTRA_GET_DELAY:
		{
			int ret;
			struct timeval tv;

			ret = gettimeofday(&tv, NULL);
			Term_xtra_long = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

			return ret;
		}

		/* React to events */
	case TERM_XTRA_REACT:
		Term_xtra_net_react();
		return (0);
	}

	/* Unknown */
	return (1);
}


/*
 * Actually MOVE the hardware cursor
 */
static errr Term_curs_net(int x, int y)
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
static errr Term_wipe_net(int x, int y, int n)
{
	char buf[2];

	term_data *td = (term_data *)(Term->data);

	buf[0] = PACKET_CLEAR;
	buf[1] = '\0';
	zsock.write_simple(net_connection, buf);

	/* Success */
	return (0);
}

/*
 * Place some text on the screen using an attribute
 */
static errr Term_text_net(int x, int y, int n, byte a, cptr s)
{
	term_data *td = (term_data *)(Term->data);
	char buf[5];

	buf[0] = PACKET_TEXT;
	buf[1] = y + 1;
	buf[2] = x + 1;
	buf[3] = a + 1;
	buf[4] = '\0';

	zsock.write_simple(net_connection, buf);
	zsock.write_simple(net_connection, s);

	buf[0] = PACKET_STOP;
	buf[1] = '\0';
	zsock.write_simple(net_connection, buf);

	/* Success */
	return (0);
}


/*
 * Create a window for the given "term_data" argument.
 *
 * Assumes legal arguments.
 */
static errr term_data_init_net(term_data *td, int rows, int cols)
{
	term *t = &td->t;

	/* Initialize the term */
	term_init(t, cols, rows, 256);

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Set some hooks */
	t->init_hook = Term_init_net;
	t->nuke_hook = Term_nuke_net;

	/* Set some more hooks */
	t->text_hook = Term_text_net;
	t->wipe_hook = Term_wipe_net;
	t->curs_hook = Term_curs_net;
	t->xtra_hook = Term_xtra_net;

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

static void net_lost_connection_hook(ip_connection *conn)
{
	printf("Lost connection ! ARGGGG\nAccepting a new one...\n");
	zsock.accept(net_serv_connection, net_connection);
	zsock.set_lose_connection(net_connection, net_lost_connection_hook);
	printf("...accepted\n");
}

/*
 * Prepare "curses" for use by the file "z-term.c"
 *
 * Installs the "hook" functions defined above, and then activates
 * the main screen "term", which clears the screen and such things.
 *
 * Someone should really check the semantics of "initscr()"
 */
errr init_net(int argc, char **argv)
{
	int i;

	int num_term = MAX_TERM_DATA, next_win = 0;
	int port = 6666;

	/* Parse args */
	for (i = 1; i < argc; i++)
	{
		if (prefix(argv[i], "-P"))
		{
			port = atoi(argv[i + 1]);
			i++;
			continue;
		}

		plog_fmt("Ignoring option: %s", argv[i]);
	}

	/* Activate hooks */
	quit_aux = hook_quit;
	core_aux = hook_quit;

	/* Create a term */
	term_data_init_net(&data[0], 25, 80);

	/* Remember the term */
	angband_term[0] = &data[0].t;

	/* Activate the "Angband" window screen */
	Term_activate(&data[0].t);

	/* Remember the active screen */
	term_screen = &data[0].t;

	/* Initialize the server and wait for thhe client */
	zsock.setup(net_serv_connection, "127.0.0.1", port, ZSOCK_TYPE_TCP, TRUE);
	zsock.open(net_serv_connection);

	printf("Accepting...\n");
	zsock.accept(net_serv_connection, net_connection);
	zsock.set_lose_connection(net_connection, net_lost_connection_hook);
	printf("...accepted\n");

	/* Success */
	return (0);
}


#endif /* USE_NET */

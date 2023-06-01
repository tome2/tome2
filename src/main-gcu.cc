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
 * To use this file, you must define "USE_GCU" in the Makefile.
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

#include "main.hpp"
#include "frontend.hpp"
#include "variable.hpp"
#include "z-util.hpp"

#include <fcntl.h>
#include <boost/algorithm/string/predicate.hpp>
#include <climits>

using boost::algorithm::starts_with;

/*
 * Hack -- play games with "bool" and "term"
 */
#undef bool

/* Avoid 'struct term' name conflict with <curses.h> (via <term.h>) on AIX */
#define term System_term

/*
 * Include the proper "header" file
 */
#ifdef USE_NCURSES
# include <ncurses.h>
#else
# include <curses.h>
#endif

#undef term

/*
 * Try redefining the colors at startup.
 */
#define REDEFINE_COLORS


/*
 * Hack -- try to guess which systems use what commands
 * Hack -- allow one of the "USE_Txxxxx" flags to be pre-set.
 * Mega-Hack -- try to guess when "POSIX" is available.
 * If the user defines two of these, we will probably crash.
 */
#if !defined(USE_TPOSIX)
# if !defined(USE_TERMIO) && !defined(USE_TCHARS)
# if defined(_POSIX_VERSION)
# define USE_TPOSIX
# else
# if defined(linux)
# define USE_TERMIO
# else
# define USE_TCHARS
# endif
# endif
# endif
#endif

/*
 * POSIX stuff
 */
#ifdef USE_TPOSIX
# include <sys/ioctl.h>
# include <termios.h>
#endif

/*
 * One version needs these files
 */
#ifdef USE_TERMIO
# include <sys/ioctl.h>
# include <termio.h>
#endif

/*
 * The other needs these files
 */
#ifdef USE_TCHARS
# include <sys/ioctl.h>
# include <sys/resource.h>
# include <sys/param.h>
# include <sys/file.h>
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>



/*
 * XXX XXX Hack -- POSIX uses "O_NONBLOCK" instead of "O_NDELAY"
 *
 * They should both work due to the "(i != 1)" test below.
 */
#ifndef O_NDELAY
# define O_NDELAY O_NONBLOCK
#endif


/*
 * OPTION: some machines lack "cbreak()"
 * On these machines, we use an older definition
 */
/* #define cbreak() crmode() */


/*
 * OPTION: some machines cannot handle "nonl()" and "nl()"
 * On these machines, we can simply ignore those commands.
 */
/* #define nonl() */
/* #define nl() */


/*
 * Save the "normal" and "angband" terminal settings
 */

#ifdef USE_TPOSIX

static struct termios norm_termios;

static struct termios game_termios;

#endif

#ifdef USE_TERMIO

static struct termio norm_termio;

static struct termio game_termio;

#endif

#ifdef USE_TCHARS

static struct ltchars norm_special_chars;
static struct sgttyb norm_ttyb;
static struct tchars norm_tchars;
static int norm_local_chars;

static struct ltchars game_special_chars;
static struct sgttyb game_ttyb;
static struct tchars game_tchars;
static int game_local_chars;

#endif

/*
 * Information about a term
 */
typedef struct term_data term_data;

struct term_data
{
	term *term_ptr;             /* All term info */

	WINDOW *win;             /* Pointer to the curses window */
};

/* Max number of windows on screen */
#define MAX_TERM_DATA 4

/* Information about our windows */
static term_data data[MAX_TERM_DATA];


/*
 * Hack -- define "A_BRIGHT" to be "A_BOLD", because on many
 * machines, "A_BRIGHT" produces ugly "inverse" video.
 */
#ifndef A_BRIGHT
# define A_BRIGHT A_BOLD
#endif

/*
 * Software flag -- we are allowed to use color
 */
static int can_use_color = false;

/*
 * Software flag -- we are allowed to change the colors
 */
static int can_fix_color = false;

/*
 * Simple Angband to Curses color conversion table
 */
static int colortable[16];



/*
 * Place the "keymap" into its "normal" state
 */
static void keymap_norm()
{

#ifdef USE_TPOSIX

	/* restore the saved values of the special chars */
	(void)tcsetattr(0, TCSAFLUSH, &norm_termios);

#endif

#ifdef USE_TERMIO

	/* restore the saved values of the special chars */
	(void)ioctl(0, TCSETA, (char *)&norm_termio);

#endif

#ifdef USE_TCHARS

	/* restore the saved values of the special chars */
	(void)ioctl(0, TIOCSLTC, (char *)&norm_special_chars);
	(void)ioctl(0, TIOCSETP, (char *)&norm_ttyb);
	(void)ioctl(0, TIOCSETC, (char *)&norm_tchars);
	(void)ioctl(0, TIOCLSET, (char *)&norm_local_chars);

#endif

}


/*
 * Place the "keymap" into the "game" state
 */
static void keymap_game()
{

#ifdef USE_TPOSIX

	/* restore the saved values of the special chars */
	(void)tcsetattr(0, TCSAFLUSH, &game_termios);

#endif

#ifdef USE_TERMIO

	/* restore the saved values of the special chars */
	(void)ioctl(0, TCSETA, (char *)&game_termio);

#endif

#ifdef USE_TCHARS

	/* restore the saved values of the special chars */
	(void)ioctl(0, TIOCSLTC, (char *)&game_special_chars);
	(void)ioctl(0, TIOCSETP, (char *)&game_ttyb);
	(void)ioctl(0, TIOCSETC, (char *)&game_tchars);
	(void)ioctl(0, TIOCLSET, (char *)&game_local_chars);

#endif

}


/*
 * Save the normal keymap
 */
static void keymap_norm_prepare()
{

#ifdef USE_TPOSIX

	/* Get the normal keymap */
	tcgetattr(0, &norm_termios);

#endif

#ifdef USE_TERMIO

	/* Get the normal keymap */
	(void)ioctl(0, TCGETA, (char *)&norm_termio);

#endif

#ifdef USE_TCHARS

	/* Get the normal keymap */
	(void)ioctl(0, TIOCGETP, (char *)&norm_ttyb);
	(void)ioctl(0, TIOCGLTC, (char *)&norm_special_chars);
	(void)ioctl(0, TIOCGETC, (char *)&norm_tchars);
	(void)ioctl(0, TIOCLGET, (char *)&norm_local_chars);

#endif

}


/*
 * Save the keymaps (normal and game)
 */
static void keymap_game_prepare()
{

#ifdef USE_TPOSIX

	/* Acquire the current mapping */
	tcgetattr(0, &game_termios);

	/* Force "Ctrl-C" to interupt */
	game_termios.c_cc[VINTR] = (char)3;

	/* Force "Ctrl-Z" to suspend */
	game_termios.c_cc[VSUSP] = (char)26;

	/* Hack -- Leave "VSTART/VSTOP" alone */

	/* Disable the standard control characters */
	game_termios.c_cc[VQUIT] = (char) - 1;
	game_termios.c_cc[VERASE] = (char) - 1;
	game_termios.c_cc[VKILL] = (char) - 1;
	game_termios.c_cc[VEOF] = (char) - 1;
	game_termios.c_cc[VEOL] = (char) - 1;

	/* Normally, block until a character is read */
	game_termios.c_cc[VMIN] = 1;
	game_termios.c_cc[VTIME] = 0;

#endif

#ifdef USE_TERMIO

	/* Acquire the current mapping */
	(void)ioctl(0, TCGETA, (char *)&game_termio);

	/* Force "Ctrl-C" to interupt */
	game_termio.c_cc[VINTR] = (char)3;

	/* Force "Ctrl-Z" to suspend */
	game_termio.c_cc[VSUSP] = (char)26;

	/* Hack -- Leave "VSTART/VSTOP" alone */

	/* Disable the standard control characters */
	game_termio.c_cc[VQUIT] = (char) - 1;
	game_termio.c_cc[VERASE] = (char) - 1;
	game_termio.c_cc[VKILL] = (char) - 1;
	game_termio.c_cc[VEOF] = (char) - 1;
	game_termio.c_cc[VEOL] = (char) - 1;

	/* Normally, block until a character is read */
	game_termio.c_cc[VMIN] = 1;
	game_termio.c_cc[VTIME] = 0;

#endif

#ifdef USE_TCHARS

	/* Get the default game characters */
	(void)ioctl(0, TIOCGETP, (char *)&game_ttyb);
	(void)ioctl(0, TIOCGLTC, (char *)&game_special_chars);
	(void)ioctl(0, TIOCGETC, (char *)&game_tchars);
	(void)ioctl(0, TIOCLGET, (char *)&game_local_chars);

	/* Force suspend (^Z) */
	game_special_chars.t_suspc = (char)26;

	/* Cancel some things */
	game_special_chars.t_dsuspc = (char) - 1;
	game_special_chars.t_rprntc = (char) - 1;
	game_special_chars.t_flushc = (char) - 1;
	game_special_chars.t_werasc = (char) - 1;
	game_special_chars.t_lnextc = (char) - 1;

	/* Force interupt (^C) */
	game_tchars.t_intrc = (char)3;

	/* Force start/stop (^Q, ^S) */
	game_tchars.t_startc = (char)17;
	game_tchars.t_stopc = (char)19;

	/* Cancel some things */
	game_tchars.t_quitc = (char) - 1;
	game_tchars.t_eofc = (char) - 1;
	game_tchars.t_brkc = (char) - 1;

#endif

}




/*
 * React to changes
 */
static void Term_xtra_gcu_react()
{

	int i;

	/* Cannot handle color redefinition */
	if (!can_fix_color)
	{
		return;
	}

	/* Set the colors */
	for (i = 0; i < 16; i++)
	{
		/* Set one color (note scaling) */
		init_color(i,
		           angband_color_table[i][1] * 1000 / 255,
		           angband_color_table[i][2] * 1000 / 255,
		           angband_color_table[i][3] * 1000 / 255);
	}

}


/*
 * User Interface for GCU.
 */
class CursesFrontend final : public Frontend {

	/*
	 * Hack -- Number of initialized "term" structures
	 */
	static int m_active;

private:
	term_data *m_term_data;

	bool event_aux(bool wait)
	{
		int i, k;

		char buf[2];

		/* Wait */
		if (wait)
		{
			/* Wait for one byte */
			i = read(0, buf, 1);

			/* Hack -- Handle bizarre "errors" */
			if ((i <= 0) && (errno != EINTR)) abort();
		}

		/* Do not wait */
		else
		{
			/* Get the current flags for stdin */
			k = fcntl(0, F_GETFL, 0);

			/* Oops */
			if (k < 0)
			{
				return false;
			}

			/* Tell stdin not to block */
			if (fcntl(0, F_SETFL, k | O_NDELAY) < 0)
			{
				return false;
			}

			/* Read one byte, if possible */
			i = read(0, buf, 1);

			/* Replace the flags for stdin */
			if (fcntl(0, F_SETFL, k))
			{
				return false;
			}
		}

		/* Ignore "invalid" keys */
		if ((i != 1) || (!buf[0]))
		{
			return false;
		}

		/* Enqueue the keypress */
		Term_keypress(buf[0]);

		/* Success */
		return true;
	}

public:
	explicit CursesFrontend(term_data *term_data)
		: m_term_data(term_data)
	{
	}

	void init() final
	{
		/* Count init's, handle first */
		if (m_active++ != 0) return;

		/* Clear */
		wclear(m_term_data->win);
		wmove(m_term_data->win, 0, 0);
		wrefresh(m_term_data->win);

		/* Game keymap */
		keymap_game();
	};

	bool icky_corner() const final
	{
		return true;
	}

	bool soft_cursor() const final
	{
		return false;
	}

	void nuke() final
	{
		/* Delete this window */
		delwin(m_term_data->win);

		/* Count nuke's, handle last */
		if (--m_active != 0) return;

		/* Reset colors to defaults */
		start_color();

		/* Get current cursor position */
		int x, y;
		getyx(curscr, y, x);

		/* Move the cursor to bottom right corner */
		mvcur(y, x, LINES - 1, 0);

		/* Flush the curses buffer */
		refresh();

		/* Exit curses */
		endwin();

		/* Flush the output */
		fflush(stdout);

		/* Normal keymap */
		keymap_norm();
	}

	void process_event(bool wait) final
	{
		event_aux(wait);
	}

	void flush_events() final
	{
		while (event_aux(false))
		{
			// Keep flushing
		}
	}

	void clear() final
	{
		touchwin(m_term_data->win);
		wclear(m_term_data->win);
	}

	void flush_output() final
	{
		wrefresh(m_term_data->win);
	}

	void noise() final
	{
		write(1, "\007", 1);
	}

	void process_queued_events() final
	{
		// No action necessary
	}

	void react() final
	{
		Term_xtra_gcu_react();
	}

	void rename_main_window(std::string_view) final
	{
		// Don't have window titles
	}

	void draw_cursor(int x, int y) final
	{
		wmove(m_term_data->win, y, x);
	}

	void draw_text(int x, int y, int n, byte a, const char *s) final
	{
		/* Set the color */
		if (can_use_color)
		{
			wattrset(m_term_data->win, colortable[a & 0x0F]);
		}

		/* Move the cursor */
		wmove(m_term_data->win, y, x);

		/* Draw each character */
		for (int i = 0; i < n; i++)
		{

			/* Draw a normal character */
			waddch(m_term_data->win, (byte)s[i]);
		}
	}

};

int CursesFrontend::m_active = 0;


/*
 * Create a window for the given "term_data" argument.
 *
 * Assumes legal arguments.
 */
static errr term_data_init_gcu(term_data *td, int rows, int cols, int y, int x)
{
	/* Create new window */
	td->win = newwin(rows, cols, y, x);

	/* Check for failure */
	if (!td->win)
	{
		/* Error */
		quit("Failed to setup curses window.");
	}

	/* Initialize the term */
	td->term_ptr = term_init(cols, rows, 256, std::make_shared<CursesFrontend>(td));

	/* Activate it */
	Term_activate(td->term_ptr);

	/* Success */
	return (0);
}


static void hook_quit(const char *)
{
	endwin();
}


/*
 * Prepare "curses" for use by the file "z-term.c"
 *
 * Installs the "hook" functions defined above, and then activates
 * the main screen "term", which clears the screen and such things.
 *
 * Someone should really check the semantics of "initscr()"
 */
int init_gcu(int argc, char **argv)
{
	int i;

	int num_term = MAX_TERM_DATA, next_win = 0;

	bool use_big_screen = false;


	/* Parse args */
	for (i = 1; i < argc; i++)
	{
		if (starts_with(argv[i], "-b"))
		{
			use_big_screen = true;
			continue;
		}

		fprintf(stderr, "Ignoring option: %s", argv[i]);
	}


	/* Extract the normal keymap */
	keymap_norm_prepare();


/* Initialize for other systems */
	if (initscr() == (WINDOW*)ERR) return ( -1);

	/* Activate hooks */
	quit_aux = hook_quit;

	/* Require standard size screen */
	if ((LINES < 24) || (COLS < 80))
	{
		quit("Angband needs at least an 80x24 'curses' screen");
	}



	/*** Init the Color-pairs and set up a translation table ***/

	/* Do we have color, and enough color, available? */
	can_use_color = ((start_color() != ERR) && has_colors() &&
	                 (COLORS >= 8) && (COLOR_PAIRS >= 8));

#ifdef REDEFINE_COLORS

	/* Can we change colors? */
	can_fix_color = (can_use_color && can_change_color() &&
	                 (COLORS >= 16) && (COLOR_PAIRS > 8));

#endif

	/* Attempt to use customized colors */
	if (can_fix_color)
	{
		/* Prepare the color pairs */
		for (i = 1; i <= 8; i++)
		{
			/* Reset the color */
			if (init_pair(i, i - 1, 0) == ERR)
			{
				quit("Color pair init failed");
			}

			/* Set up the colormap */
			colortable[i - 1] = (COLOR_PAIR(i) | A_NORMAL);
			colortable[i + 7] = (COLOR_PAIR(i) | A_BRIGHT);
		}

		/* Take account of "gamma correction" XXX XXX XXX */

		/* Prepare the "Angband Colors" */
		Term_xtra_gcu_react();
	}

	/* Attempt to use colors */
	else if (can_use_color)
	{
		/* Color-pair 0 is *always* WHITE on BLACK */

		/* Prepare the color pairs */
		init_pair(1, COLOR_RED, COLOR_BLACK);
		init_pair(2, COLOR_GREEN, COLOR_BLACK);
		init_pair(3, COLOR_YELLOW, COLOR_BLACK);
		init_pair(4, COLOR_BLUE, COLOR_BLACK);
		init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(6, COLOR_CYAN, COLOR_BLACK);
		init_pair(7, COLOR_BLACK, COLOR_BLACK);

		/* Prepare the "Angband Colors" -- Bright white is too bright */
		colortable[0] = (COLOR_PAIR(7) | A_NORMAL); 	/* Black */
		colortable[1] = (COLOR_PAIR(0) | A_NORMAL); 	/* White */
		colortable[2] = (COLOR_PAIR(6) | A_NORMAL); 	/* Grey XXX */
		colortable[3] = (COLOR_PAIR(1) | A_BRIGHT); 	/* Orange XXX */
		colortable[4] = (COLOR_PAIR(1) | A_NORMAL); 	/* Red */
		colortable[5] = (COLOR_PAIR(2) | A_NORMAL); 	/* Green */
		colortable[6] = (COLOR_PAIR(4) | A_NORMAL); 	/* Blue */
		colortable[7] = (COLOR_PAIR(3) | A_NORMAL); 	/* Umber */
		colortable[8] = (COLOR_PAIR(7) | A_BRIGHT); 	/* Dark-grey XXX */
		colortable[9] = (COLOR_PAIR(6) | A_BRIGHT); 	/* Light-grey XXX */
		colortable[10] = (COLOR_PAIR(5) | A_NORMAL); 	/* Purple */
		colortable[11] = (COLOR_PAIR(3) | A_BRIGHT); 	/* Yellow */
		colortable[12] = (COLOR_PAIR(5) | A_BRIGHT); 	/* Light Red XXX */
		colortable[13] = (COLOR_PAIR(2) | A_BRIGHT); 	/* Light Green */
		colortable[14] = (COLOR_PAIR(4) | A_BRIGHT); 	/* Light Blue */
		colortable[15] = (COLOR_PAIR(3) | A_NORMAL); 	/* Light Umber XXX */
	}


	/*** Low level preparation ***/


	/* Prepare */
	raw();
	noecho();
	nonl();

	/* Extract the game keymap */
	keymap_game_prepare();


	/*** Now prepare the term(s) ***/

	/* Big screen -- one big term */
	if (use_big_screen)
	{
		/* Create a term */
		term_data_init_gcu(&data[0], LINES, COLS, 0, 0);

		/* Remember the term */
		angband_term[0] = data[0].term_ptr;
	}

	/* No big screen -- create as many term windows as possible */
	else
	{
		/* Create several terms */
		for (i = 0; i < num_term; i++)
		{
			int rows, cols, y, x;

			/* Decide on size and position */
			switch (i)
			{
				/* Upper left */
			case 0:
				{
					rows = 24;
					cols = 80;
					y = x = 0;
					break;
				}

				/* Lower left */
			case 1:
				{
					rows = LINES - 25;
					cols = 80;
					y = 25;
					x = 0;
					break;
				}

				/* Upper right */
			case 2:
				{
					rows = 24;
					cols = COLS - 81;
					y = 0;
					x = 81;
					break;
				}

				/* Lower right */
			case 3:
				{
					rows = LINES - 25;
					cols = COLS - 81;
					y = 25;
					x = 81;
					break;
				}

				/* XXX */
			default:
				{
					rows = cols = y = x = 0;
					break;
				}
			}

			/* Skip non-existant windows */
			if (rows <= 0 || cols <= 0) continue;

			/* Create a term */
			term_data_init_gcu(&data[next_win], rows, cols, y, x);

			/* Remember the term */
			angband_term[next_win] = data[next_win].term_ptr;

			/* One more window */
			next_win++;
		}
	}

	/* Activate the "Angband" window screen */
	Term_activate(data[0].term_ptr);

	/* Remember the active screen */
	angband_term[0] = data[0].term_ptr;

	/* Success */
	return (0);
}

int main(int argc, char *argv[])
{
	return main_real(
		argc,
		argv,
		"gcu",
		init_gcu,
		"  -- -b              Requests big screen\n");
}

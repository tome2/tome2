/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

/* Purpose: a generic, efficient, terminal window package -BEN- */

#include "z-term.hpp"

#include "key_queue.hpp"
#include "tome/unique_handle.hpp"

#include <cassert>
#include <memory>
#include <optional>
#include <vector>


/*
 * This file provides a generic, efficient, terminal window package,
 * which can be used not only on standard terminal environments such
 * as dumb terminals connected to a Unix box, but also in more modern
 * "graphic" environments, such as the Macintosh or Unix/X11.
 *
 * Each "window" works like a standard "dumb terminal", that is, it
 * can display a two dimensional array of grids containing colored
 * textual symbols, plus an optional cursor, and it can be used to
 * get keypress events from the user.
 *
 * In fact, this package can simply be used, if desired, to support
 * programs which will look the same on a dumb terminal as they do
 * on a graphic platform such as the Macintosh.
 *
 * This package was designed to help port the game "Angband" to a wide
 * variety of different platforms.  Angband, like many other games in
 * the "rogue-like" heirarchy, requires, at the minimum, the ability
 * to display "colored textual symbols" in a standard 80x24 "window",
 * such as that provided by most dumb terminals, and many old personal
 * computers, and to check for "keypresses" from the user.  The major
 * concerns were thus portability and efficiency, so Angband could be
 * easily ported to many different systems, with minimal effort, and
 * yet would run quickly on each of these systems, no matter what kind
 * of underlying hardware/software support was being used.
 *
 * It is important to understand the differences between the older
 * "dumb terminals" and the newer "graphic interface" machines, since
 * this package was designed to work with both types of systems.
 *
 * New machines:
 *   waiting for a keypress is complex
 *   checking for a keypress is often cheap
 *   changing "colors" may be expensive
 *   the "color" of a "blank" is rarely important
 *   moving the "cursor" is relatively cheap
 *   use a "software" cursor (only moves when requested)
 *   drawing characters normally will not erase old ones
 *   drawing a character on the cursor often erases it
 *   may have fast routines for "clear a region"
 *   the bottom right corner is usually not special
 *
 * Old machines:
 *   waiting for a keypress is simple
 *   checking for a keypress is often expensive
 *   changing "colors" is usually cheap
 *   the "color" of a "blank" may be important
 *   moving the "cursor" may be expensive
 *   use a "hardware" cursor (moves during screen updates)
 *   drawing new symbols automatically erases old ones
 *   characters may only be drawn at the cursor location
 *   drawing a character on the cursor will move the cursor
 *   may have fast routines for "clear entire window"
 *   may have fast routines for "clear to end of line"
 *   the bottom right corner is often dangerous
 *
 *
 * This package provides support for multiple windows, each of an
 * arbitrary size (up to 255x255), each with its own set of flags,
 * and its own hooks to handle several low-level procedures which
 * differ from platform to platform.  Then the main program simply
 * creates one or more "term" structures, setting the various flags
 * and hooks in a manner appropriate for the current platform, and
 * then it can use the various "term" structures without worrying
 * about the underlying platform.
 *
 *
 * This package allows each "grid" in each window to hold an attr/char
 * pair, with each ranging from 0 to 255, and makes very few assumptions
 * about the meaning of any attr/char values.  We assume that "attr 0" is
 * "black", with the semantics that "black" text should be
 * sent to "Term_wipe()" instead of "Term_text()".
 *
 * Finally, we use a special attr/char pair, defaulting to "attr 0" and
 * "char 32", also known as "black space", when we "erase" or "clear"
 * any window, but this pair can be redefined to any pair, including
 * the standard "white space", or the bizarre "emptiness" ("attr 0"
 * and "char 0"), as long as various obscure restrictions are met.
 *
 *
 * This package provides several functions which allow a program to
 * interact with the "term" structures.  Most of the functions allow
 * the program to "request" certain changes to the current "term",
 * such as moving the cursor, drawing an attr/char pair, erasing a
 * region of grids, hiding the cursor, etc.  Then there is a special
 * function which causes all of the "pending" requests to be performed
 * in an efficient manner.  There is another set of functions which
 * allow the program to query the "requested state" of the current
 * "term", such as asking for the cursor location, or what attr/char
 * is at a given location, etc.  There is another set of functions
 * dealing with "keypress" events, which allows the program to ask if
 * the user has pressed any keys, or to forget any keys the user pressed.
 * There is a pair of functions to allow this package to memorize the
 * contents of the current "term", and to restore these contents at
 * a later time.  There is a special function which allows the program
 * to specify which "term" structure should be the "current" one.  At
 * the lowest level, there is a set of functions which allow a new
 * "term" to be initialized or destroyed, and which allow this package,
 * or a program, to access the special "hooks" defined for the current
 * "term", and a set of functions which those "hooks" can use to inform
 * this package of the results of certain occurances, for example, one
 * such function allows this package to learn about user keypresses,
 * detected by one of the special "hooks".
 *
 * We provide, among other things, the functions "Term_keypress()"
 * to "react" to keypress events, and "Term_redraw()" to redraw the
 * entire window, plus "Term_resize()" to note a new size.
 *
 *
 * Note that the current "term" contains two "window images".  One of
 * these images represents the "requested" contents of the "term", and
 * the other represents the "actual" contents of the "term", at the time
 * of the last performance of pending requests.  This package uses these
 * two images to determine the "minimal" amount of work needed to make
 * the "actual" contents of the "term" match the "requested" contents of
 * the "term".  This method is not perfect, but it often reduces the
 * amount of work needed to perform the pending requests, which thus
 * increases the speed of the program itself.  This package promises
 * that the requested changes will appear to occur either "all at once"
 * or in a "top to bottom" order.  In addition, a "cursor" is maintained,
 * and this cursor is updated along with the actual window contents.
 *
 * Currently, the "Term_fresh()" routine attempts to perform the "minimum"
 * number of physical updates, in terms of total "work" done by the hooks
 * Term_wipe(), Term_text(), and Term_pict(), making use of the fact that
 * adjacent characters of the same color can both be drawn together using
 * the "Term_text()" hook, and that "black" text can often be sent to the
 * "Term_wipe()" hook instead of the "Term_text()" hook, and if something
 * is already displayed in a window, then it is not necessary to display
 * it again.  Unfortunately, this may induce slightly non-optimal results
 * in some cases, in particular, those in which, say, a string of ten
 * characters needs to be written, but the fifth character has already
 * been displayed.  Currently, this will cause the "Term_text()" routine
 * to be called once for each half of the string, instead of once for the
 * whole string, which, on some machines, may be non-optimal behavior.
 *
 * The new formalism includes a "displayed" screen image (old) which
 * is actually seen by the user, a "requested" screen image (scr)
 * which is being prepared for display, a "memorized" screen image
 * (mem) which is used to save and restore screen images.
 *
 *
 * Several "flags" are available in each "term" to allow the underlying
 * visual system (which initializes the "term" structure) to "optimize"
 * the performance of this package for the given system, or to request
 * certain behavior which is helpful/required for the given system.
 *
 * The "soft_cursor" flag indicates the use of a "soft" cursor, which
 * only moves when explicitly requested,and which is "erased" when
 * any characters are drawn on top of it.  This flag is used for all
 * "graphic" systems which handle the cursor by "drawing" it.
 *
 * The "icky_corner" flag indicates that the bottom right "corner"
 * of the windows are "icky", and "printing" anything there may
 * induce "messy" behavior, such as "scrolling".  This flag is used
 * for most old "dumb terminal" systems.
 *
 *
 * The "term" structure contains the following function "hooks":
 *
 *   Term->init_hook = Init the term
 *   Term->nuke_hook = Nuke the term
 *   Term->xtra_hook = Perform extra actions
 *   Term->curs_hook = Draw (or Move) the cursor
 *   Term->text_hook = Draw some text in the window
 *
 * The "Term->xtra_hook" hook provides a variety of different functions,
 * based on the first parameter (which should be taken from the various
 * TERM_XTRA_* defines) and the second parameter (which may make sense
 * only for some first parameters).  It is available to the program via
 * the "Term_xtra()" function, though some first parameters are only
 * "legal" when called from inside this package.
 *
 * The "Term->curs_hook" hook provides this package with a simple way
 * to "move" or "draw" the cursor to the grid "x,y", depending on the
 * setting of the "soft_cursor" flag.  Note that the cursor is never
 * redrawn if "nothing" has happened to the screen (even temporarily).
 * This hook is required.
 *
 * The "Term->text_hook" hook provides this package with a simple way
 * to "draw", starting at "x,y", the "n" chars contained in "cp", using
 * the attr "a".  This hook assumes that the input is valid, and that
 * "n" is between 1 and 256 inclusive, but it should NOT assume that
 * the contents of "cp" are null-terminated.
 *
 * The game "Angband" uses a set of files called "main-xxx.c", for
 * various "xxx" suffixes.  Most of these contain a function called
 * "init_xxx()", that will prepare the underlying visual system for
 * use with Angband, and then create one or more "term" structures,
 * using flags and hooks appropriate to the given platform, so that
 * the "main()" function can call one (or more) of the "init_xxx()"
 * functions, as appropriate, to prepare the required "term" structs
 * (one for each desired sub-window), and these "init_xxx()" functions
 * are called from a centralized "main()" function in "main.c".  Other
 * "main-xxx.c" systems contain their own "main()" function which, in
 * addition to doing everything needed to initialize the actual program,
 * also does everything that the normal "init_xxx()" functions would do.
 *
 * The game "Angband" defines, in addition to "attr 0", all of the
 * attr codes from 1 to 15, using definitions in "defines.h", and
 * thus the "main-xxx.c" files used by Angband must handle these
 * attr values correctly.  Also, they must handle all other attr
 * values, though they may do so in any way they wish, for example,
 * by always taking every attr code mod 16.  Many of the "main-xxx.c"
 * files use "white space" ("attr 1" / "char 32") to "erase" or "clear"
 * any window, for efficiency.
 *
 * See "main-xxx.c" for a simple skeleton file which can be used to
 * create a "visual system" for a new platform when porting Angband.
 */



/*
 * A term_win is a "window" for a Term
 *
 *	- Cursor Useless/Visible codes
 *	- Cursor Location (see "Useless")
 *
 *	- Array[h] -- Access to the attribute array
 *	- Array[h] -- Access to the character array
 *
 *	- Array[h*w] -- Attribute array
 *	- Array[h*w] -- Character array
 *
 * Note that the attr/char pair at (x,y) is a[y][x]/c[y][x]
 * and that the row of attr/chars at (0,y) is a[y]/c[y]
 */

struct term_win final
{
private:
	std::vector<byte> va;
	std::vector<char> vc;

public:

	bool cu = true;
	bool cv = false;
	byte cx = 0;
	byte cy = 0;

	std::vector<byte *> a;
	std::vector<char *> c;

	/**
	 * Ctor
	 */
	explicit term_win(int w, int h)
		: va(h * w)
		, vc(h * w)
		, a(h)
		, c(h)
	{
		// The idea here is that our va and vc vectors are just
		// a single contiguous block of memory and that each "row"
		// of a and c point into the correct location in that single
		// block of memory.
		for (int y = 0; y < h; y++)
		{
			a[y] = va.data() + w * y;
			c[y] = vc.data() + w * y;
		}
	}

	/*
	 * Copy contents of a "term_win" up to the given dimensions.
	 */
	void copy_from(term_win const *f, int w, int h)
	{
		/* Copy contents */
		for (int y = 0; y < h; y++)
		{
			byte *f_aa = f->a[y];
			char *f_cc = f->c[y];

			byte *s_aa = a[y];
			char *s_cc = c[y];

			for (int x = 0; x < w; x++)
			{
				*s_aa++ = *f_aa++;
				*s_cc++ = *f_cc++;
			}
		}

		/* Copy cursor */
		cx = f->cx;
		cy = f->cy;
		cu = f->cu;
		cv = f->cv;
	}

	/**
	 * Copy contents of a "term_win" up to the given dimensions.
	 */
	void copy_from(std::unique_ptr<term_win> const &p, int w, int h)
	{
		copy_from(p.get(), w, h);
	}

	/**
	 * Force cursor to be visible.
	 */
	void set_cursor_visible()
	{
		cu = false;
		cv = true;
	}

	/**
	 * Dtor
	 */
	~term_win() = default;

};



static errr push_result_to_errr(key_queue::push_result_t r)
{
	using pr = key_queue::push_result_t;

	switch (r)
	{
	case pr::OK:
		return 0;
	case pr::OVERFLOW:
		return 1;
	}
}



/*
 * An actual "term" structure
 *
 *	- Extra "data" info (used by implementation)
 *
 *
 *	- Flag "active_flag"
 *	  This "term" is "active"
 *
 *	- Flag "mapped_flag"
 *	  This "term" is "mapped"
 *
 *	- Flag "total_erase"
 *	  This "term" should be fully erased
 *
 *	- Flag "icky_corner"
 *	  This "term" has an "icky" corner grid
 *
 *	- Flag "soft_cursor"
 *	  This "term" uses a "software" cursor
 *
 *
 *
 *
 *	- Ignore this pointer
 *
 *	- Keypress Queue -- various data
 *
 *	- Keypress Queue -- pending keys
 *
 *
 *	- Window Width (max 255)
 *	- Window Height (max 255)
 *
 *	- Minimum modified row
 *	- Maximum modified row
 *
 *	- Minimum modified column (per row)
 *	- Maximum modified column (per row)
 *
 *
 *	- Displayed screen image
 *	- Requested screen image
 *
 *	- Temporary screen image
 *	- Memorized screen image
 *
 *
 *	- Hook for init-ing the term
 *	- Hook for nuke-ing the term
 *
 *	- Hook for extra actions
 *
 *	- Hook for placing the cursor
 *
 *	- Hook for drawing a string of chars using an attr
 *
 *	- Hook for drawing a sequence of special attr/char pairs
 */

struct term
{
	void *data;

	bool active_flag = false;
	bool mapped_flag = false;
	bool total_erase = true;
	bool icky_corner = false;
	bool soft_cursor = false;

	key_queue m_key_queue;

	byte wid;
	byte hgt;

	byte y1;
	byte y2;

	std::vector<byte> x1;
	std::vector<byte> x2;

	std::unique_ptr<term_win> old;
	std::unique_ptr<term_win> scr;
	std::unique_ptr<term_win> mem;

	init_hook_t *init_hook = nullptr;
	nuke_hook_t *nuke_hook = nullptr;
	xtra_hook_t *xtra_hook = nullptr;
	curs_hook_t *curs_hook = nullptr;
	text_hook_t *text_hook = nullptr;

	resize_hook_t *resize_hook = nullptr;

	/**
	 * Ctor
	 */
	term(int w, int h, int k, void *data_)
		: data(data_)
		, m_key_queue(k)
		, wid(w)
		, hgt(h)
		, x1(h)
		, x2(h)
	{
		/* Allocate "displayed" */
		old = std::make_unique<term_win>(w, h);

		/* Allocate "requested" */
		scr = std::make_unique<term_win>(w, h);

		/* Assume change */
		for (int y = 0; y < h; y++)
		{
			/* Assume change */
			x1[y] = 0;
			x2[y] = w - 1;
		}

		/* Assume change */
		y1 = 0;
		y2 = h - 1;
	}

	/**
	 * Dtor
	 */
	~term()
	{
		/* Hack -- Call the special "nuke" hook */
		if (active_flag)
		{
			/* Call the "nuke" hook */
			if (nuke_hook)
			{
				(*nuke_hook)(data);
			}

			/* Remember */
			active_flag = false;

			/* Assume not mapped */
			mapped_flag = false;
		}
	}

};



/*
 * The current "term"
 */
term *Term = nullptr;

/*** External hooks ***/


/*
 * Execute the "Term->xtra_hook" hook, if any.
 */
void Term_xtra(int n, int v)
{
	if (Term->xtra_hook)
	{
		(*Term->xtra_hook)(Term->data, n, v);
	}
}


/*** Efficient routines ***/


/*
 * Mentally draw an attr/char at a given location
 *
 * Assumes given location and values are valid.
 */
void Term_queue_char(int x, int y, byte a, char c)
{
	auto const &scrn = Term->scr;

	byte *scr_aa = &scrn->a[y][x];
	char *scr_cc = &scrn->c[y][x];

	/* Hack -- Ignore non-changes */
	if ((*scr_aa == a) && (*scr_cc == c)) return;

	/* Save the "literal" information */
	*scr_aa = a;
	*scr_cc = c;

	/* Check for new min/max row info */
	if (y < Term->y1) Term->y1 = y;
	if (y > Term->y2) Term->y2 = y;

	/* Check for new min/max col info for this row */
	if (x < Term->x1[y]) Term->x1[y] = x;
	if (x > Term->x2[y]) Term->x2[y] = x;
}



/*
 * Mentally draw some attr/chars at a given location
 *
 * Assumes that (x,y) is a valid location, that the first "n" characters
 * of the string "s" are all valid (non-zero), and that (x+n-1,y) is also
 * a valid location, so the first "n" characters of "s" can all be added
 * starting at (x,y) without causing any illegal operations.
 */
static void Term_queue_chars(int x, int y, int n, byte a, const char *s)
{
	int x1 = -1, x2 = -1;

	byte *scr_aa = Term->scr->a[y];
	char *scr_cc = Term->scr->c[y];

	/* Queue the attr/chars */
	for ( ; n; x++, s++, n--)
	{
		int oa = scr_aa[x];
		int oc = scr_cc[x];

		/* Hack -- Ignore non-changes */
		if ((oa == a) && (oc == *s)) continue;

		/* Save the "literal" information */
		scr_aa[x] = a;
		scr_cc[x] = *s;

		/* Note the "range" of window updates */
		if (x1 < 0) x1 = x;
		x2 = x;
	}

	/* Expand the "change area" as needed */
	if (x1 >= 0)
	{
		/* Check for new min/max row info */
		if (y < Term->y1) Term->y1 = y;
		if (y > Term->y2) Term->y2 = y;

		/* Check for new min/max col info in this row */
		if (x1 < Term->x1[y]) Term->x1[y] = x1;
		if (x2 > Term->x2[y]) Term->x2[y] = x2;
	}
}


/*
 * Blank attribute/character
 */
static const byte ATTR_BLANK = TERM_WHITE;
static const char CHAR_BLANK = ' ';

/*
 * Call the text hook
 */
static void do_text_hook(int x, int y, int n, byte a, const char *s)
{
	assert(Term->text_hook);
	(*Term->text_hook)(Term->data, x, y, n, a, s);
}

/*
 * Call the curs hook
 */
static void do_curs_hook(int x, int y)
{
	assert(Term->curs_hook);
	(*Term->curs_hook)(Term->data, x, y);
}


/*
 * Flush a row of the current window (see "Term_fresh")
 *
 * Display text using "Term_text()" and "Term_wipe()"
 */
static void Term_fresh_row_text(int y, int x1, int x2)
{
	int x;

	byte *old_aa = Term->old->a[y];
	char *old_cc = Term->old->c[y];

	byte *scr_aa = Term->scr->a[y];
	char *scr_cc = Term->scr->c[y];

	/* Pending length */
	int fn = 0;

	/* Pending start */
	int fx = 0;

	/* Pending attr */
	byte fa = ATTR_BLANK;

	byte oa;
	char oc;

	byte na;
	char nc;

	/* Scan "modified" columns */
	for (x = x1; x <= x2; x++)
	{
		/* See what is currently here */
		oa = old_aa[x];
		oc = old_cc[x];

		/* See what is desired there */
		na = scr_aa[x];
		nc = scr_cc[x];

		/* Handle unchanged grids */
		if ((na == oa) && (nc == oc))
		{
			/* Flush */
			if (fn)
			{
				do_text_hook(fx, y, fn, fa, &scr_cc[fx]);

				/* Forget */
				fn = 0;
			}

			/* Skip */
			continue;
		}

		/* Save new contents */
		old_aa[x] = na;
		old_cc[x] = nc;

		/* Notice new color */
		if (fa != na)
		{
			/* Flush */
			if (fn)
			{
				/* Draw the pending chars */
				do_text_hook(fx, y, fn, fa, &scr_cc[fx]);

				/* Forget */
				fn = 0;
			}

			/* Save the new color */
			fa = na;
		}

		/* Restart and Advance */
		if (fn++ == 0) fx = x;
	}

	/* Flush */
	if (fn)
	{
		do_text_hook(fx, y, fn, fa, &scr_cc[fx]);
	}
}





/*
 * Actually perform all requested changes to the window
 *
 * If absolutely nothing has changed, not even temporarily, or if the
 * current "Term" is not mapped, then this function will return 1 and
 * do absolutely nothing.
 *
 * Note that when "soft_cursor" is true, we erase the cursor (if needed)
 * whenever anything has changed, and redraw it (if needed) after all of
 * the screen updates are complete.  This will induce a small amount of
 * "cursor flicker" but only when the screen has been updated.  If the
 * screen is updated and then restored, you may still get this flicker.
 *
 * When "soft_cursor" is not true, we make the cursor invisible before
 * doing anything else if it is supposed to be invisible by the time we
 * are done, and we make it visible after moving it to its final location
 * after all of the screen updates are complete.
 *
 * Note that "Term_xtra(TERM_XTRA_CLEAR,0)" must erase the entire screen,
 * including the cursor, if needed, and may place the cursor anywhere.
 *
 * Note that "Term_xtra(TERM_XTRA_FRESH,0)" will be called after
 * all of the rows have been "flushed".
 *
 * The helper functions currently "skip" any grids which already contain
 * the desired contents.  This may or may not be the best method, especially
 * when the desired content fits nicely into the current stripe.  For example,
 * it might be better to go ahead and queue them while allowed, but keep a
 * count of the "trailing skipables", then, when time to flush, or when a
 * "non skippable" is found, force a flush if there are too many skippables.
 *
 * Perhaps an "initialization" stage, where the "text" (and "attr")
 * buffers are "filled" with information, converting "blanks" into
 * a convenient representation, and marking "skips" with "zero chars",
 * and then some "processing" is done to determine which chars to skip.
 *
 * Currently, the helper functions are optimal for systems which prefer
 * to "print a char + move a char + print a char" to "print three chars",
 * and for applications that do a lot of "detailed" color printing.
 *
 * In the two "queue" functions, total "non-changes" are "pre-skipped".
 * The helper functions must also handle situations in which the contents
 * of a grid are changed, but then changed back to the original value,
 * and situations in which two grids in the same row are changed, but
 * the grids between them are unchanged.
 *
 * Normally, the "Term_wipe()" function is used only to display "blanks"
 * that were induced by "Term_clear()" or "Term_erase()", and then only
 * if the "attr_blank" and "char_blank" fields have not been redefined
 * to use "white space" instead of the default "black space".  Actually,
 * the "Term_wipe()" function is used to display all "black" text, such
 * as the default "spaces" created by "Term_clear()" and "Term_erase()".
 *
 * Note that if no "black" text is ever drawn, and if "attr_blank" is
 * not "zero", then the "Term_wipe" hook will never be used.
 *
 * This function does nothing unless the "Term" is "mapped", which allows
 * certain systems to optimize the handling of "closed" windows.
 *
 * On systems with a "soft" cursor, we must explicitly erase the cursor
 * before flushing the output, if needed, to prevent a "jumpy" refresh.
 * The actual method for this is horrible, but there is very little that
 * we can do to simplify it efficiently.  XXX XXX XXX
 *
 * On systems with a "hard" cursor, we will "hide" the cursor before
 * flushing the output, if needed, to avoid a "flickery" refresh.  It
 * would be nice to *always* hide the cursor during the refresh, but
 * this might be expensive (and/or ugly) on some machines.
 *
 * The "Term->icky_corner" flag is used to avoid calling "Term_wipe()"
 * or "Term_pict()" or "Term_text()" on the bottom right corner of the
 * window, which might induce "scrolling" or other nasty stuff on old
 * dumb terminals.  This flag is handled very efficiently.  We assume
 * that the "Term_curs()" call will prevent placing the cursor in the
 * corner, if needed, though I doubt such placement is ever a problem.
 * Currently, the use of "Term->icky_corner" and "Term->soft_cursor"
 * together may result in undefined behavior.
 */
void Term_fresh()
{
	int x, y;

	int w = Term->wid;
	int h = Term->hgt;

	int y1 = Term->y1;
	int y2 = Term->y2;

	auto const &old = Term->old;
	auto const &scr = Term->scr;


	/* Do nothing unless "mapped" */
	if (!Term->mapped_flag)
	{
		return;
	}


	/* Trivial Refresh */
	if ((y1 > y2) &&
	                (scr->cu == old->cu) &&
	                (scr->cv == old->cv) &&
	                (scr->cx == old->cx) &&
	                (scr->cy == old->cy) &&
	                !(Term->total_erase))
	{
		/* Nothing */
		return;
	}

	/* Handle "total erase" */
	if (Term->total_erase)
	{
		byte na = ATTR_BLANK;
		char nc = CHAR_BLANK;

		/* Physically erase the entire window */
		Term_xtra(TERM_XTRA_CLEAR, 0);

		/* Hack -- clear all "cursor" data */
		old->cv = false;
		old->cu = false;
		old->cx = 0;
		old->cy = 0;

		/* Wipe each row */
		for (y = 0; y < h; y++)
		{
			byte *aa = old->a[y];
			char *cc = old->c[y];

			/* Wipe each column */
			for (x = 0; x < w; x++)
			{
				/* Wipe each grid */
				*aa++ = na;
				*cc++ = nc;
			}
		}

		/* Redraw every row */
		Term->y1 = y1 = 0;
		Term->y2 = y2 = h - 1;

		/* Redraw every column */
		for (y = 0; y < h; y++)
		{
			Term->x1[y] = 0;
			Term->x2[y] = w - 1;
		}

		/* Forget "total erase" */
		Term->total_erase = false;
	}


	/* Cursor update -- Erase old Cursor */
	if (Term->soft_cursor)
	{
		/* Cursor was visible */
		if (!old->cu && old->cv)
		{
			int tx = old->cx;
			int ty = old->cy;

			byte *old_aa = old->a[ty];
			char *old_cc = old->c[ty];

			byte oa = old_aa[tx];
			char oc = old_cc[tx];

			/* Hack -- restore the actual character */
			do_text_hook(tx, ty, 1, oa, &oc);
		}
	}

	/* Something to update */
	if (y1 <= y2)
	{
		/* Handle "icky corner" */
		if (Term->icky_corner)
		{
			/* Avoid the corner */
			if (y2 >= h - 1)
			{
				/* Avoid the corner */
				if (Term->x2[h - 1] > w - 2)
				{
					/* Avoid the corner */
					Term->x2[h - 1] = w - 2;
				}
			}
		}


		/* Scan the "modified" rows */
		for (y = y1; y <= y2; ++y)
		{
			int x1 = Term->x1[y];
			int x2 = Term->x2[y];

			/* Flush each "modified" row */
			if (x1 <= x2)
			{
				/* Flush the row */
				Term_fresh_row_text(y, x1, x2);

				/* This row is all done */
				Term->x1[y] = w;
				Term->x2[y] = 0;
			}
		}

		/* No rows are invalid */
		Term->y1 = h;
		Term->y2 = 0;
	}


	/* Cursor update -- Show new Cursor */
	if (Term->soft_cursor)
	{
		/* Draw the cursor */
		if (!scr->cu && scr->cv)
		{
			/* Call the cursor display routine */
			do_curs_hook(scr->cx, scr->cy);
		}
	}

	/* Cursor Update -- Show new Cursor */
	else
	{
		/* The cursor is useless, hide it */
		if (scr->cu)
		{
			/* Paranoia -- Put the cursor NEAR where it belongs */
			do_curs_hook(w - 1, scr->cy);

			/* Make the cursor invisible */
			/* Term_xtra(TERM_XTRA_SHAPE, 0); */
		}

		/* The cursor is invisible, hide it */
		else if (!scr->cv)
		{
			/* Paranoia -- Put the cursor where it belongs */
			do_curs_hook(scr->cx, scr->cy);

			/* Make the cursor invisible */
			/* Term_xtra(TERM_XTRA_SHAPE, 0); */
		}

		/* The cursor is visible, display it correctly */
		else
		{
			/* Put the cursor where it belongs */
			do_curs_hook(scr->cx, scr->cy);
		}
	}


	/* Save the "cursor state" */
	old->cu = scr->cu;
	old->cv = scr->cv;
	old->cx = scr->cx;
	old->cy = scr->cy;


	/* Actually flush the output */
	Term_xtra(TERM_XTRA_FRESH, 0);
}



/*** Output routines ***/


/*
 * Place the cursor at a given location
 *
 * Note -- "illegal" requests do not move the cursor.
 */
errr Term_gotoxy(int x, int y)
{
	int w = Term->wid;
	int h = Term->hgt;

	/* Verify */
	if ((x < 0) || (x >= w)) return ( -1);
	if ((y < 0) || (y >= h)) return ( -1);

	/* Remember the cursor */
	Term->scr->cx = x;
	Term->scr->cy = y;

	/* The cursor is not useless */
	Term->scr->cu = false;

	/* Success */
	return (0);
}


/*
 * At a given location, place an attr/char
 * Do not change the cursor position
 * No visual changes until "Term_fresh()".
 */
void Term_draw(int x, int y, byte a, char c)
{
	int w = Term->wid;
	int h = Term->hgt;

	/* Verify location */
	if ((x < 0) || (x >= w)) return;
	if ((y < 0) || (y >= h)) return;

	/* Paranoia -- illegal char */
	if (!c) return;

	/* Queue it for later */
	Term_queue_char(x, y, a, c);
}


/*
 * Using the given attr, add the given char at the cursor.
 *
 * We return "-2" if the character is "illegal". XXX XXX
 *
 * We return "-1" if the cursor is currently unusable.
 *
 * We queue the given attr/char for display at the current
 * cursor location, and advance the cursor to the right,
 * marking it as unuable and returning "1" if it leaves
 * the screen, and otherwise returning "0".
 *
 * So when this function, or the following one, return a
 * positive value, future calls to either function will
 * return negative ones.
 */
void Term_addch(byte a, char c)
{
	int w = Term->wid;

	/* Handle "unusable" cursor */
	if (Term->scr->cu)
	{
		return;
	}

	/* Paranoia -- no illegal chars */
	if (!c)
	{
		return;
	}

	/* Queue the given character for display */
	Term_queue_char(Term->scr->cx, Term->scr->cy, a, c);

	/* Advance the cursor */
	Term->scr->cx++;

	/* Success */
	if (Term->scr->cx < w)
	{
		return;
	}

	/* Note "Useless" cursor */
	Term->scr->cu = true;
}


/*
 * At the current location, using an attr, add a string
 *
 * We also take a length "n", using negative values to imply
 * the largest possible value, and then we use the minimum of
 * this length and the "actual" length of the string as the
 * actual number of characters to attempt to display, never
 * displaying more characters than will actually fit, since
 * we do NOT attempt to "wrap" the cursor at the screen edge.
 *
 * We return "-1" if the cursor is currently unusable.
 * We return "N" if we were "only" able to write "N" chars,
 * even if all of the given characters fit on the screen,
 * and mark the cursor as unusable for future attempts.
 *
 * So when this function, or the preceding one, return a
 * positive value, future calls to either function will
 * return negative ones.
 */
errr Term_addstr(int n, byte a, const char *s)
{
	int k;

	int w = Term->wid;

	errr res = 0;

	/* Handle "unusable" cursor */
	if (Term->scr->cu) return ( -1);

	/* Obtain maximal length */
	k = (n < 0) ? (w + 1) : n;

	/* Obtain the usable string length */
	for (n = 0; (n < k) && s[n]; n++) /* loop */;

	/* React to reaching the edge of the screen */
	if (Term->scr->cx + n >= w) res = n = w - Term->scr->cx;

	/* Queue the first "n" characters for display */
	Term_queue_chars(Term->scr->cx, Term->scr->cy, n, a, s);

	/* Advance the cursor */
	Term->scr->cx += n;

	/* Hack -- Notice "Useless" cursor */
	if (res)
	{
		Term->scr->cu = true;
	}

	/* Success (usually) */
	return (res);
}


/*
 * Move to a location and, using an attr, add a char
 */
void Term_putch(int x, int y, byte a, char c)
{
	/* Move first */
	if (Term_gotoxy(x, y)) return;

	/* Add the char */
	Term_addch(a, c);
}


/*
 * Move to a location and, using an attr, add a string
 */
void Term_putstr(int x, int y, int n, byte a, const char *s)
{
	/* Move first */
	if (Term_gotoxy(x, y)) return;

	/* Add the string */
	Term_addstr(n, a, s);
}



/*
 * Place cursor at (x,y), and clear the next "n" chars
 */
void Term_erase(int x, int y, int n)
{
	int i;

	int w = Term->wid;
	/* int h = Term->hgt; */

	int x1 = -1;
	int x2 = -1;

	int na = ATTR_BLANK;
	int nc = CHAR_BLANK;

	byte *scr_aa;
	char *scr_cc;

	/* Place cursor */
	if (Term_gotoxy(x, y))
	{
		return;
	}

	/* Force legal size */
	if (x + n > w) n = w - x;

	/* Fast access */
	scr_aa = Term->scr->a[y];
	scr_cc = Term->scr->c[y];

	if (n > 0 && (byte)scr_cc[x] == 255 && scr_aa[x] == 255)
	{
		x--;
		n++;
	}

	/* Scan every column */
	for (i = 0; i < n; i++, x++)
	{
		int oa = scr_aa[x];
		int oc = scr_cc[x];

		/* Hack -- Ignore "non-changes" */
		if ((oa == na) && (oc == nc)) continue;

		/* Save the "literal" information */
		scr_aa[x] = na;
		scr_cc[x] = nc;

		/* Track minimum changed column */
		if (x1 < 0) x1 = x;

		/* Track maximum changed column */
		x2 = x;
	}

	/* Expand the "change area" as needed */
	if (x1 >= 0)
	{
		/* Check for new min/max row info */
		if (y < Term->y1) Term->y1 = y;
		if (y > Term->y2) Term->y2 = y;

		/* Check for new min/max col info in this row */
		if (x1 < Term->x1[y]) Term->x1[y] = x1;
		if (x2 > Term->x2[y]) Term->x2[y] = x2;
	}
}


/*
 * Clear the entire window, and move to the top left corner
 *
 * Note the use of the special "total_erase" code
 */
void Term_clear()
{
	int x, y;

	int w = Term->wid;
	int h = Term->hgt;

	byte na = ATTR_BLANK;
	char nc = CHAR_BLANK;

	/* Cursor usable */
	Term->scr->cu = false;

	/* Cursor to the top left */
	Term->scr->cx = Term->scr->cy = 0;

	/* Wipe each row */
	for (y = 0; y < h; y++)
	{
		byte *scr_aa = Term->scr->a[y];
		char *scr_cc = Term->scr->c[y];

		/* Wipe each column */
		for (x = 0; x < w; x++)
		{
			scr_aa[x] = na;
			scr_cc[x] = nc;
		}

		/* This row has changed */
		Term->x1[y] = 0;
		Term->x2[y] = w - 1;
	}

	/* Every row has changed */
	Term->y1 = 0;
	Term->y2 = h - 1;

	/* Force "total erase" */
	Term->total_erase = true;
}


/*
 * Redraw (and refresh) the whole window.
 */
void Term_redraw()
{
	/* Force "total erase" */
	Term->total_erase = true;

	/* Hack -- Refresh */
	Term_fresh();
}


/*
 * Redraw part of a window.
 */
void Term_redraw_section(int x1, int y1, int x2, int y2)
{
	int i, j;

	char *c_ptr;

	/* Bounds checking */
	if (y2 >= Term->hgt) y2 = Term->hgt - 1;
	if (x2 >= Term->wid) x2 = Term->wid - 1;
	if (y1 < 0) y1 = 0;
	if (x1 < 0) x1 = 0;

	/* Set y limits */
	Term->y1 = y1;
	Term->y2 = y2;

	/* Set the x limits */
	for (i = Term->y1; i <= Term->y2; i++)
	{
		Term->x1[i] = x1;
		Term->x2[i] = x2;

		c_ptr = Term->old->c[i];

		/* Clear the section so it is redrawn */
		for (j = x1; j <= x2; j++)
		{
			/* Hack - set the old character to "none" */
			c_ptr[j] = 0;
		}
	}

	/* Hack -- Refresh */
	Term_fresh();
}

void Term_bell()
{
	Term_xtra(TERM_XTRA_NOISE, 0);
}

/*** Access routines ***/

namespace { // anonymous

/**
 * Policy handler for setting/resetting cursor visibility.
 */
struct CursorVisibilyPolicy {
public:
	struct handle_type {
		term *t = nullptr;
		bool cv = false;
	};

	static handle_type from(term *t)
	{
		return handle_type {
			.t = t,
			.cv = t->scr->cv
		};
	}

	static handle_type get_null()
	{
		return handle_type();
	}

	static bool is_null(handle_type const &v)
	{
		return v.t == nullptr;
	}

	static void close(handle_type const &v)
	{
		Term->scr->cv = v.cv;
	}
};

/**
 * Policy handler for setting/restoring cursor flags.
 */
struct CursorFlagsPolicy {

public:
	struct handle_type {
		term *t = nullptr;
		bool cu = false;
		bool cv = false;
	};

private:
	handle_type handle;

public:
	static handle_type from(term *t)
	{
		return handle_type {
			.t = t,
			.cu = t->scr->cu,
			.cv = t->scr->cv
		};
	}

	static handle_type get_null()
	{
		return handle_type();
	}

	static bool is_null(handle_type const &h)
	{
		return h.t == nullptr;
	}

	static void close(handle_type &h)
	{
		h.t->scr->cu = h.cu;
		h.t->scr->cv = h.cv;
	}

};

} // namespace (anonymous)


void Term_with_saved_cursor_flags(std::function<void ()> callback)
{
	unique_handle<CursorFlagsPolicy> resetter(CursorFlagsPolicy::from(Term));
	callback();
}

void Term_with_saved_cursor_visbility(std::function<void ()> callback)
{
	unique_handle<CursorVisibilyPolicy> resetter(CursorVisibilyPolicy::from(Term));
	callback();
}


/*
 * Extract the current window size
 */
void Term_get_size(int *w, int *h)
{
	term_get_size(Term, w, h);
}


/*
 * Extract the current cursor location
 */
void Term_locate(int *x, int *y)
{
	/* Access the cursor */
	(*x) = Term->scr->cx;
	(*y) = Term->scr->cy;
}


/*
 * At a given location, determine the "current" attr and char
 * Note that this refers to what will be on the window after the
 * next call to "Term_fresh()".  It may or may not already be there.
 */
void Term_what(int x, int y, byte *a, char *c)
{
	int w = Term->wid;
	int h = Term->hgt;

	/* Verify location */
	assert(x >= 0);
	assert(x < w);
	assert(y >= 0);
	assert(y < h);

	/* Direct access */
	(*a) = Term->scr->a[y][x];
	(*c) = Term->scr->c[y][x];
}


void Term_hide_cursor()
{
	Term->scr->cv = 0;
}


void Term_show_cursor()
{
	Term->scr->set_cursor_visible();
}



/*** Input routines ***/


/*
 * Flush and forget the input
 */
void Term_flush()
{
	/* Hack -- Flush all events */
	Term_xtra(TERM_XTRA_FLUSH, 0);

	/* Forget all keypresses */
	Term->m_key_queue.clear();
}



/*
 * Add a keypress to the "queue"
 */
void Term_keypress(int k)
{
	/* Ignore non-keys */
	if (!k) return;

	/* Push */
	Term->m_key_queue.push_back(k);
}


/*
 * Add a keypress to the FRONT of the "queue"
 */
errr Term_key_push(int k)
{
	/* Hack -- Refuse to enqueue non-keys */
	if (!k) return ( -1);

	/* Push */
	return push_result_to_errr(Term->m_key_queue.push_front(k));
}


/*
 * Check for a pending keypress on the key queue.
 *
 * Store the keypress, if any, in "ch", and return "0".
 * Otherwise store "zero" in "ch", and return "1".
 *
 * Wait for a keypress if "wait" is true.
 *
 * Remove the keypress if "take" is true.
 */
errr Term_inkey(char *ch, bool wait, bool take)
{
	auto &key_queue = Term->m_key_queue;

	/* Assume no key */
	(*ch) = '\0';

	/* Process queued UI events */
	Term_xtra(TERM_XTRA_BORED, 0);

	/* Wait */
	if (wait)
	{
		/* Process pending events while necessary */
		while (key_queue.empty())
		{
			/* Process events (wait for one) */
			Term_xtra(TERM_XTRA_EVENT, true);
		}
	}

	/* Do not Wait */
	else
	{
		/* Process pending events if necessary */
		if (key_queue.empty())
		{
			/* Process events (do not wait) */
			Term_xtra(TERM_XTRA_EVENT, false);
		}
	}

	/* No keys are ready */
	if (key_queue.empty())
	{
		return (1);
	}

	/* Extract the next keypress */
	if (take)
	{
		*ch = key_queue.pop_front();
	}
	else
	{
		*ch = key_queue.front();
	}

	/* Success */
	return (0);
}


/*** Extra routines ***/


/*
 * Save the "requested" screen into the "memorized" screen
 *
 * Every "Term_save()" should match exactly one "Term_load()"
 */
void Term_save()
{
	int w = Term->wid;
	int h = Term->hgt;

	/* Create */
	if (!Term->mem)
	{
		Term->mem = std::make_unique<term_win>(w, h);
	}

	/* Grab */
	Term->mem->copy_from(Term->scr, w, h);
}

/*
 * Same as before but can save more than once
 */
term_win* Term_save_to()
{
	int w = Term->wid;
	int h = Term->hgt;

	/* Copy */
	auto save = new term_win(w, h);
	save->copy_from(Term->scr, w, h);

	/* Success */
	return (save);
}

/*
 * Restore the "requested" contents (see above).
 *
 * Every "Term_save()" should match exactly one "Term_load()"
 */
void Term_load()
{
	int w = Term->wid;
	int h = Term->hgt;

	/* Create empty contents if nothing was actually saved previously */
	if (!Term->mem)
	{
		Term->mem = std::make_unique<term_win>(w, h);
	}

	/* Load */
	Term->scr->copy_from(Term->mem, w, h);

	/* Assume change */
	for (int y = 0; y < h; y++)
	{
		/* Assume change */
		Term->x1[y] = 0;
		Term->x2[y] = w - 1;
	}

	/* Assume change */
	Term->y1 = 0;
	Term->y2 = h - 1;
}

/*
 * Same as previous but allow to save more than one
 */
void Term_load_from(term_win *save)
{
	int y;

	int w = Term->wid;
	int h = Term->hgt;

	/* Create */
	if (!save)
	{
		return;
	}

	/* Load */
	Term->scr->copy_from(save, w, h);

	/* Assume change */
	for (y = 0; y < h; y++)
	{
		/* Assume change */
		Term->x1[y] = 0;
		Term->x2[y] = w - 1;
	}

	/* Assume change */
	Term->y1 = 0;
	Term->y2 = h - 1;

	/* Free is requested */
	delete save;
}

/*
 * React to a new physical window size.
 */
void Term_resize(int w, int h)
{
	int i;

	int wid, hgt;

	/* Ignore illegal changes */
	if ((w < 1) || (h < 1))
	{
		return;
	}

	/* Ignore non-changes */
	if ((Term->wid == w) && (Term->hgt == h))
	{
		return;
	}

	/* Minimum dimensions */
	wid = std::min<int>(Term->wid, w);
	hgt = std::min<int>(Term->hgt, h);

	/* Create new window */
	{
		auto hold_old = std::move(Term->old);
		Term->old = std::make_unique<term_win>(w, h);
		Term->old->copy_from(hold_old, wid, hgt);

		/* Illegal cursor? */
		if (Term->old->cx >= w) Term->old->cu = true;
		if (Term->old->cy >= h) Term->old->cu = true;
	}

	/* Create new window */
	{
		auto hold_scr = std::move(Term->scr);
		Term->scr = std::make_unique<term_win>(w, h);
		Term->scr->copy_from(hold_scr, wid, hgt);

		/* Illegal cursor? */
		if (Term->scr->cx >= w) Term->scr->cu = true;
		if (Term->scr->cy >= h) Term->scr->cu = true;
	}

	/* Create new window */
	if (Term->mem)
	{
		auto hold_mem = std::move(Term->mem);
		Term->mem = std::make_unique<term_win>(w, h);
		Term->mem->copy_from(hold_mem, wid, hgt);

		/* Illegal cursor? */
		if (Term->mem->cx >= w) Term->mem->cu = true;
		if (Term->mem->cy >= h) Term->mem->cu = true;
	}

	/* Resize scanners */
	Term->x1.resize(h);
	Term->x2.resize(h);
	for (i = 0; i < h; i++)
	{
		Term->x1[i] = 0;
		Term->x2[i] = w - 1;
	}

	/* Save new size */
	Term->wid = w;
	Term->hgt = h;

	/* Force "total erase" */
	Term->total_erase = true;

	/* Assume change */
	Term->y1 = 0;
	Term->y2 = h - 1;

	/* Execute the "resize_hook" hook, if available */
	if (Term->resize_hook)
	{
		Term->resize_hook();
	}
}



/*
 * Activate a new Term (and deactivate the current Term)
 *
 * This function is extremely important, and also somewhat bizarre.
 * It is the only function that should "modify" the value of "Term".
 *
 * To "create" a valid "term", one should do "term_init(t)", then
 * set the various flags and hooks, and then do "Term_activate(t)".
 */
void Term_activate(term *t)
{
	/* No change? */
	if (Term == t)
	{
		return;
	}

	/* Deactivate the old Term */
	if (Term) Term_xtra(TERM_XTRA_LEVEL, 0);

	/* Hack -- Call the special "init" hook */
	if (t && !t->active_flag)
	{
		/* Call the "init" hook */
		if (t->init_hook)
		{
			(*t->init_hook)(t->data);
		}

		/* Remember */
		t->active_flag = true;

		/* Assume mapped */
		t->mapped_flag = true;
	}

	/* Remember the Term */
	Term = t;

	/* Activate the new Term */
	if (Term) Term_xtra(TERM_XTRA_LEVEL, 1);
}


/**
 * Set the current terminal "mapped" flag.
 */
void Term_mapped()
{
	Term->mapped_flag = true;
}


/**
 * Unset the current terminal "mapped" flag.
 */
void Term_unmapped()
{
	Term->mapped_flag = false;
}


/*
 * Nuke a term
 */
void term_nuke(term *t)
{
	delete t;
}


/*
 * Initialize a term, using a window of the given size.
 * Also prepare the "input queue" for "k" keypresses
 * By default, the cursor starts out "invisible"
 * By default, we "erase" using "black spaces"
 */
term *term_init(void *data, int w, int h, int k)
{
	return new term(w, h, k, data);
}

void term_init_icky_corner(term *t)
{
	t->icky_corner = true;
}

void term_init_soft_cursor(term *t)
{
	t->soft_cursor = true;
}

void term_init_ui_hooks(term *t, term_ui_hooks_t hooks)
{
	t->init_hook = hooks.init_hook;
	t->nuke_hook = hooks.nuke_hook;
	t->xtra_hook = hooks.xtra_hook;
	t->curs_hook = hooks.curs_hook;
	t->text_hook = hooks.text_hook;
}

void term_set_resize_hook(term *t, resize_hook_t *hook)
{
	t->resize_hook = hook;
}

void term_get_size(term *t, int *w, int *h)
{
	if (w)
	{
		(*w) = t->wid;
	}

	if (h)
	{
		(*h) = t->hgt;
	}
}

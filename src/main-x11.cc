/* File: main-x11.c */

/*
 * Copyright (c) 1997 Ben Harrison, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


/*
 * This file helps Angband work with UNIX/X11 computers.
 *
 * To use this file, compile with "USE_X11" defined, and link against all
 * the various "X11" libraries which may be needed.
 *
 * See also "main-xaw.c".
 *
 * Part of this file provides a user interface package composed of several
 * pseudo-objects, including "metadpy" (a display), "infowin" (a window),
 * "infoclr" (a color), and "infofnt" (a font).  Actually, the package was
 * originally much more interesting, but it was bastardized to keep this
 * file simple.
 *
 * The rest of this file is an implementation of "main-xxx.c" for X11.
 *
 * Most of this file is by Ben Harrison (benh@phial.com).
 */

/*
 * The following shell script can be used to launch Angband, assuming that
 * it was extracted into "~/Angband", and compiled using "USE_X11", on a
 * Linux machine, with a 1280x1024 screen, using 6 windows (with the given
 * characteristics), with gamma correction of 1.8 -> (1 / 1.8) * 256 = 142,
 * and without graphics (add "-g" for graphics).  Just copy this comment
 * into a file, remove the leading " * " characters (and the head/tail of
 * this comment), and make the file executable.
 *
 *
 * #!/bin/csh
 *
 * # Describe attempt
 * echo "Launching angband..."
 * sleep 2
 *
 * # Main window
 * setenv ANGBAND_X11_FONT_0 10x20
 * setenv ANGBAND_X11_AT_X_0 5
 * setenv ANGBAND_X11_AT_Y_0 510
 *
 * # Message window
 * setenv ANGBAND_X11_FONT_1 8x13
 * setenv ANGBAND_X11_AT_X_1 5
 * setenv ANGBAND_X11_AT_Y_1 22
 * setenv ANGBAND_X11_ROWS_1 35
 *
 * # Inventory window
 * setenv ANGBAND_X11_FONT_2 8x13
 * setenv ANGBAND_X11_AT_X_2 635
 * setenv ANGBAND_X11_AT_Y_2 182
 * setenv ANGBAND_X11_ROWS_3 23
 *
 * # Equipment window
 * setenv ANGBAND_X11_FONT_3 8x13
 * setenv ANGBAND_X11_AT_X_3 635
 * setenv ANGBAND_X11_AT_Y_3 22
 * setenv ANGBAND_X11_ROWS_3 12
 *
 * # Monster recall window
 * setenv ANGBAND_X11_FONT_4 6x13
 * setenv ANGBAND_X11_AT_X_4 817
 * setenv ANGBAND_X11_AT_Y_4 847
 * setenv ANGBAND_X11_COLS_4 76
 * setenv ANGBAND_X11_ROWS_4 11
 *
 * # Object recall window
 * setenv ANGBAND_X11_FONT_5 6x13
 * setenv ANGBAND_X11_AT_X_5 817
 * setenv ANGBAND_X11_AT_Y_5 520
 * setenv ANGBAND_X11_COLS_5 76
 * setenv ANGBAND_X11_ROWS_5 24
 *
 * # The build directory
 * cd ~/Angband
 *
 * # Gamma correction
 * setenv ANGBAND_X11_GAMMA 142
 *
 * # Launch Angband
 * ./src/angband -mx11 -- -n6 &
 *
 */

#include "config.hpp"
#include "defines.hpp"
#include "frontend.hpp"
#include "loadsave.hpp"
#include "main.hpp"
#include "util.hpp"
#include "variable.hpp"
#include "z-form.hpp"
#include "z-util.hpp"

#ifndef __MAKEDEPEND__
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xatom.h>
#endif /* __MAKEDEPEND__ */

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <climits>

#include <sys/time.h>
#include <boost/algorithm/string/predicate.hpp>

using boost::algorithm::equals;
using boost::algorithm::starts_with;

#define FORCELOWER(A)  ((isupper((A))) ? tolower((A)) : (A))

/*
 * This file is designed to be "included" by "main-x11.c" or "main-xaw.c",
 * which will have already "included" several relevant header files.
 */

#ifndef IsModifierKey

/*
 * Keysym macros, used on Keysyms to test for classes of symbols
 * These were stolen from one of the X11 header files
 *
 * Also appears in "main-x11.c".
 */

#define IsKeypadKey(keysym) \
(((unsigned)(keysym) >= XK_KP_Space) && ((unsigned)(keysym) <= XK_KP_Equal))

#define IsCursorKey(keysym) \
(((unsigned)(keysym) >= XK_Home)     && ((unsigned)(keysym) <  XK_Select))

#define IsPFKey(keysym) \
(((unsigned)(keysym) >= XK_KP_F1)     && ((unsigned)(keysym) <= XK_KP_F4))

#define IsFunctionKey(keysym) \
(((unsigned)(keysym) >= XK_F1)       && ((unsigned)(keysym) <= XK_F35))

#define IsMiscFunctionKey(keysym) \
(((unsigned)(keysym) >= XK_Select)   && ((unsigned)(keysym) <  XK_KP_Space))

#define IsModifierKey(keysym) \
(((unsigned)(keysym) >= XK_Shift_L)  && ((unsigned)(keysym) <= XK_Hyper_R))

#endif /* IsModifierKey */


/*
 * Checks if the keysym is a special key or a normal key
 * Assume that XK_MISCELLANY keysyms are special
 *
 * Also appears in "main-x11.c".
 */
#define IsSpecialKey(keysym) \
((unsigned)(keysym) >= 0xFF00)


/*
 * Hack -- Convert an RGB value to an X11 Pixel, or die.
 *
 * Original code by Desvignes Sebastien (desvigne@solar12.eerie.fr).
 *
 * BMP format support by Denis Eropkin (denis@dream.homepage.ru).
 *
 * Major fixes and cleanup by Ben Harrison (benh@phial.com).
 */
static unsigned long create_pixel(Display *dpy, byte red, byte green, byte blue)
{
	Colormap cmap = DefaultColormapOfScreen(DefaultScreenOfDisplay(dpy));

	char cname[8];

	XColor xcolour;

	/* Build the color */

	xcolour.red = red * 255 + red;
	xcolour.green = green * 255 + green;
	xcolour.blue = blue * 255 + blue;
	xcolour.flags = DoRed | DoGreen | DoBlue;

	/* Attempt to Allocate the Parsed color */
	if (!(XAllocColor(dpy, cmap, &xcolour)))
	{
		quit_fmt("Couldn't allocate bitmap color '%s'\n", cname);
	}

	return (xcolour.pixel);
}




/*
 * Notes on Colors:
 *
 *   1) On a monochrome (or "fake-monochrome") display, all colors
 *   will be "cast" to "fg," except for the bg color, which is,
 *   obviously, cast to "bg".  Thus, one can ignore this setting.
 *
 *   2) Because of the inner functioning of the color allocation
 *   routines, colors may be specified as (a) a typical color name,
 *   (b) a hexidecimal color specification (preceded by a pound sign),
 *   or (c) by strings such as "fg", "bg", "zg".
 *
 *   3) Due to the workings of the init routines, many colors
 *   may also be dealt with by their actual pixel values.  Note that
 *   the pixel with all bits set is "zg = (1<<metadpy->depth)-1", which
 *   is not necessarily either black or white.
 */



/**** Generic Types ****/


/*
 * An X11 pixell specifier
 */
typedef unsigned long Pixell;

/*
 * The structures defined below
 */
typedef struct metadpy metadpy;
typedef struct infowin infowin;
typedef struct infoclr infoclr;
typedef struct infofnt infofnt;


/*
 * A structure summarizing a given Display.
 *
 *	- The Display itself
 *	- The default Screen for the display
 *	- The virtual root (usually just the root)
 *	- The default colormap (from a macro)
 *
 *	- The "name" of the display
 *
 *	- The socket to listen to for events
 *
 *	- The width of the display screen (from a macro)
 *	- The height of the display screen (from a macro)
 *	- The bit depth of the display screen (from a macro)
 *
 *	- The black Pixell (from a macro)
 *	- The white Pixell (from a macro)
 *
 *	- The background Pixell (default: black)
 *	- The foreground Pixell (default: white)
 *	- The maximal Pixell (Equals: ((2 ^ depth)-1), is usually ugly)
 *
 *	- Bit Flag: Force all colors to black and white (default: !color)
 *	- Bit Flag: Allow the use of color (default: depth > 1)
 *	- Bit Flag: We created 'dpy', and so should nuke it when done.
 */
struct metadpy
{
	Display *dpy;
	Screen *screen;
	Window root;
	Colormap cmap;

	char *name;

	int fd;

	unsigned int width;
	unsigned int height;
	unsigned int depth;

	Pixell black;
	Pixell white;

	Pixell bg;
	Pixell fg;
	Pixell zg;

unsigned int mono:
	1;
unsigned int color:
	1;
unsigned int nuke:
	1;
};



/*
 * A Structure summarizing Window Information.
 *
 * I assume that a window is at most 30000 pixels on a side.
 * I assume that the root windw is also at most 30000 square.
 *
 *	- The Window
 *	- The current Input Event Mask
 *
 *	- The location of the window
 *	- The width, height of the window
 *	- The border width of this window
 *
 *	- Byte: 1st Extra byte
 *
 *	- Bit Flag: This window is currently Mapped
 *	- Bit Flag: This window needs to be redrawn
 *	- Bit Flag: This window has been resized
 *
 *	- Bit Flag: We should nuke 'win' when done with it
 *
 *	- Bit Flag: 1st extra flag
 *	- Bit Flag: 2nd extra flag
 *	- Bit Flag: 3rd extra flag
 *	- Bit Flag: 4th extra flag
 */
struct infowin
{
	Window win;
	long mask;

	s16b ox, oy;

	s16b x, y;
	s16b w, h;
	u16b b;

	byte byte1;

unsigned int mapped:
	1;
unsigned int redraw:
	1;
unsigned int resize:
	1;

unsigned int nuke:
	1;

unsigned int flag1:
	1;
unsigned int flag2:
	1;
unsigned int flag3:
	1;
unsigned int flag4:
	1;
};






/*
 * A Structure summarizing Operation+Color Information
 *
 *	- The actual GC corresponding to this info
 *
 *	- The Foreground Pixell Value
 *	- The Background Pixell Value
 *
 *	- Num (0-15): The operation code (As in Clear, Xor, etc)
 *	- Bit Flag: The GC is in stipple mode
 *	- Bit Flag: Destroy 'gc' at Nuke time.
 */
struct infoclr
{
	GC gc;

	Pixell fg;
	Pixell bg;

	unsigned int code: 4;
	unsigned int nuke: 1;
};



/*
 * A Structure to Hold Font Information
 *
 *	- The 'XFontStruct*' (yields the 'Font')
 *
 *	- The font name
 *
 *	- The default character width
 *	- The default character height
 *	- The default character ascent
 *
 *	- Byte: Pixel offset used during fake mono
 *
 *	- Flag: Force monospacing via 'wid'
 *	- Flag: Nuke info when done
 */
struct infofnt
{
	XFontStruct *info;

	const char *name;

	s16b wid;
	s16b twid;
	s16b hgt;
	s16b asc;

	byte off;

unsigned int mono:
	1;
unsigned int nuke:
	1;
};



/**** Generic Globals ****/


/*
 * The "default" values
 */
static metadpy metadpy_default;


/*
 * The "current" variables
 */
static metadpy *Metadpy = &metadpy_default;


/**** Generic code ****/

/*
 * Simple routine to save the state of the game when the display connection
 * is broken. Remember, you cannot do anything in this function that will
 * generate X protocol requests.
 */
int x_io_error_handler(Display *d)
{
	/* We have nothing to save */
	if (!character_generated) return 0;

	save_dungeon();
	save_player();

	return 0;
}

/*
 * Create a new metadpy and initialize it.
 *
 * Inputs:
 *	name: The name of the Display
 *
 * Notes:
 *	If 'name' is NULL, but 'dpy' is set, extract name from dpy
 *	If 'dpy' is NULL, then Create the named Display
 *	If 'name' is NULL, and so is 'dpy', use current Display
 *
 * Return -1 on error.
 */
static errr Metadpy_new(const char *name)
{
	metadpy *m = Metadpy;

	/*** Create the display ***/

	Display *dpy = XOpenDisplay(name);
	if (!dpy) return ( -1);

	m->nuke = 1;

	XSetIOErrorHandler(x_io_error_handler);

	/*** Save some information ***/

	/* Save the Display itself */
	m->dpy = dpy;

	/* Get the Screen and Virtual Root Window */
	m->screen = DefaultScreenOfDisplay(dpy);
	m->root = RootWindowOfScreen(m->screen);

	/* Get the default colormap */
	m->cmap = DefaultColormapOfScreen(m->screen);

	/* Extract the true name of the display */
	m->name = DisplayString(dpy);

	/* Extract the fd */
	m->fd = ConnectionNumber(Metadpy->dpy);

	/* Save the Size and Depth of the screen */
	m->width = WidthOfScreen(m->screen);
	m->height = HeightOfScreen(m->screen);
	m->depth = DefaultDepthOfScreen(m->screen);

	/* Save the Standard Colors */
	m->black = BlackPixelOfScreen(m->screen);
	m->white = WhitePixelOfScreen(m->screen);

	/*** Make some clever Guesses ***/

	/* Guess at the desired 'fg' and 'bg' Pixell's */
	m->bg = m->black;
	m->fg = m->white;

	/* Calculate the Maximum allowed Pixel value.  */
	m->zg = (1 << m->depth) - 1;

	/* Save various default Flag Settings */
	m->color = ((m->depth > 1) ? 1 : 0);
	m->mono = ((m->color) ? 0 : 1);

	/* Return "success" */
	return (0);
}


/*
 * Set the name (in the title bar) of Infowin
 */
static void Infowin_set_name(infowin *iwin, std::string_view name_sv)
{
	char buf[128];

	// Trim to the size of the buffer - 1 so that
	// strncpy is guaranteed to NUL-terminate.
	auto name = name_sv.substr(0, sizeof(buf) - 1);

	// Copy
	strncpy(buf, name.begin(), name.size());
	buf[name.size()] = '\0';

	char *bp = buf;

	// Set
	XTextProperty tp;
	Status st = XStringListToTextProperty(&bp, 1, &tp);
	if (st)
	{
		XSetWMName(Metadpy->dpy, iwin->win, &tp);
	}
}


/*
 * Prepare a new 'infowin'.
 */
static errr Infowin_prepare(infowin *iwin, Window xid)
{
	Window tmp_win;
	XWindowAttributes xwa;
	int x, y;
	unsigned int w, h, b, d;

	/* Assign stuff */
	iwin->win = xid;

	/* Check For Error XXX Extract some ACTUAL data from 'xid' */
	XGetGeometry(Metadpy->dpy, xid, &tmp_win, &x, &y, &w, &h, &b, &d);

	/* Apply the above info */
	iwin->x = x;
	iwin->y = y;
	iwin->w = w;
	iwin->h = h;
	iwin->b = b;

	/* Check Error XXX Extract some more ACTUAL data */
	XGetWindowAttributes(Metadpy->dpy, xid, &xwa);

	/* Apply the above info */
	iwin->mask = xwa.your_event_mask;
	iwin->mapped = ((xwa.map_state == IsUnmapped) ? 0 : 1);

	/* And assume that we are exposed */
	iwin->redraw = 1;

	/* Success */
	return (0);
}


/*
 * Init an infowin by giving some data.
 *
 * Inputs:
 *	dad: The Window that should own this Window (if any)
 *	x,y: The position of this Window
 *	w,h: The size of this Window
 *	b,d: The border width and pixel depth
 *
 * Notes:
 *	If 'dad == None' assume 'dad == root'
 */
static errr Infowin_init_data(infowin *iwin, Window dad, int x, int y, int w, int h,
                              int b, Pixell fg, Pixell bg)
{
	Window xid;

	/* Wipe it clean */
	memset(iwin, 0, sizeof(struct infowin));


	/*** Error Check XXX ***/


	/*** Create the Window 'xid' from data ***/

	/* What happened here?  XXX XXX XXX */

	/* If no parent given, depend on root */
	if (dad == None)

		/* #ifdef USE_GRAPHICS

				xid = XCreateWindow(Metadpy->dpy, Metadpy->root, x, y, w, h, b, 8, InputOutput, CopyFromParent, 0, 0);

			else
		*/

		/* #else */

		dad = Metadpy->root;

	/* #endif */

	/* Create the Window XXX Error Check */
	xid = XCreateSimpleWindow(Metadpy->dpy, dad, x, y, w, h, b, fg, bg);

	/* Start out selecting No events */
	XSelectInput(Metadpy->dpy, xid, 0L);


	/*** Prepare the new infowin ***/

	/* Mark it as nukable */
	iwin->nuke = 1;

	/* Attempt to Initialize the infowin */
	return Infowin_prepare(iwin, xid);
}



/*
 * Modify the event mask of an Infowin
 */
static void Infowin_set_mask(infowin *iwin, long mask)
{
	/* Save the new setting */
	iwin->mask = mask;

	/* Execute the Mapping */
	XSelectInput(Metadpy->dpy, iwin->win, iwin->mask);
}


/*
 * Request that Infowin be mapped
 */
static void Infowin_map(infowin *iwin)
{
	/* Execute the Mapping */
	XMapWindow(Metadpy->dpy, iwin->win);
}


/*
 * Request that Infowin be raised
 */
static void Infowin_raise(infowin *iwin)
{
	/* Raise towards visibility */
	XRaiseWindow(Metadpy->dpy, iwin->win);
}


/*
 * Request that Infowin be moved to a new location
 */
static void Infowin_impell(infowin *iwin, int x, int y)
{
	XMoveWindow(Metadpy->dpy, iwin->win, x, y);
}


/*
 * Visually clear Infowin
 */
static void Infowin_wipe(infowin *iwin)
{
	XClearWindow(Metadpy->dpy, iwin->win);
}

/*
 * Opcodes for Infoclr_init_data().
 */
static constexpr int Infoclr_Opcode_CPY = 3;
static constexpr int Infoclr_Opcode_XOR = 6;

/*
 * Initialize an infoclr with some data
 *
 * Inputs:
 *	fg:   The Pixell for the requested Foreground (see above)
 *	bg:   The Pixell for the requested Background (see above)
 *	op:   The Opcode for the requested Operation (see above)
 *	stip: The stipple mode
 */
static errr Infoclr_init_data(infoclr *iclr, Pixell fg, Pixell bg, int op)
{
	GC gc;
	XGCValues gcv;
	unsigned long gc_mask;



	/*** Simple error checking of opr and clr ***/

	/* Check the 'Pixells' for realism */
	if (bg > Metadpy->zg) return ( -1);
	if (fg > Metadpy->zg) return ( -1);

	/* Check the data for trueness */
	if ((op < 0) || (op > 15)) return ( -1);


	/*** Create the requested 'GC' ***/

	/* Assign the proper GC function */
	gcv.function = op;

	/* Assign the proper GC background */
	gcv.background = bg;

	/* Assign the proper GC foreground */
	gcv.foreground = fg;

	/* Hack -- Handle XOR (xor is code 6) by hacking bg and fg */
	if (op == Infoclr_Opcode_XOR)
	{
		gcv.background = 0;
		gcv.foreground = (bg ^ fg);
	}

	/* Assign the proper GC Fill Style */
	gcv.fill_style = FillSolid;

	/* Turn off 'Give exposure events for pixmap copying' */
	gcv.graphics_exposures = False;

	/* Set up the GC mask */
	gc_mask = (GCFunction | GCBackground | GCForeground |
	           GCFillStyle | GCGraphicsExposures);

	/* Create the GC detailed above */
	gc = XCreateGC(Metadpy->dpy, Metadpy->root, gc_mask, &gcv);


	/*** Initialize ***/

	/* Wipe the iclr clean */
	memset(iclr, 0, sizeof(struct infoclr));

	/* Assign the GC */
	iclr->gc = gc;

	/* Nuke it when done */
	iclr->nuke = 1;

	/* Assign the parms */
	iclr->fg = fg;
	iclr->bg = bg;
	iclr->code = op;

	/* Success */
	return (0);
}



/*
 * Change the 'fg' for an infoclr
 *
 * Inputs:
 *	fg:   The Pixell for the requested Foreground (see above)
 */
static errr Infoclr_change_fg(infoclr *iclr, Pixell fg)
{
	/*** Simple error checking of opr and clr ***/

	/* Check the 'Pixells' for realism */
	if (fg > Metadpy->zg) return ( -1);


	/*** Change ***/

	/* Change */
	XSetForeground(Metadpy->dpy, iclr->gc, fg);

	/* Success */
	return (0);
}



/*
 * Prepare a new 'infofnt'
 */
static errr Infofnt_prepare(infofnt *ifnt, XFontStruct *info)
{
	/* Assign the struct */
	ifnt->info = info;

	/* Jump into the max bouonds thing */
	auto cs = &(info->max_bounds);

	/* Extract default sizing info */
	ifnt->asc = info->ascent;
	ifnt->hgt = info->ascent + info->descent;
	ifnt->wid = cs->width;
	ifnt->twid = ifnt->wid;

	/* Success */
	return (0);
}


/*
 * Init an infofnt by its Name
 *
 * Inputs:
 *	name: The name of the requested Font
 */
static errr Infofnt_init_data(infofnt *ifnt, const char *name)
{
	XFontStruct *info;


	/*** Load the info Fresh, using the name ***/

	/* If the name is not given, report an error */
	if (!name) return ( -1);

	/* Attempt to load the font */
	info = XLoadQueryFont(Metadpy->dpy, name);

	/* The load failed, try to recover */
	if (!info) return ( -1);


	/*** Init the font ***/

	/* Wipe the thing */
	memset(ifnt, 0, sizeof(struct infofnt));

	/* Attempt to prepare it */
	if (Infofnt_prepare(ifnt, info))
	{
		/* Free the font */
		XFreeFont(Metadpy->dpy, info);

		/* Fail */
		return ( -1);
	}

	/* Save a copy of the font name */
	ifnt->name = strdup(name);

	/* Mark it as nukable */
	ifnt->nuke = 1;

	/* Success */
	return (0);
}


/*
 * Standard Text
 */
static void Infofnt_text_std(infowin *iwin, infoclr *iclr, infofnt *ifnt, int x, int y, const char *str, int len)
{
	/*** Do a brief info analysis ***/

	/* Do nothing if the string is null */
	if (!str || !*str)
	{
		return;
	}

	/* Get the length of the string */
	if (len < 0) len = strlen(str);


	/*** Decide where to place the string, vertically ***/

	/* Ignore Vertical Justifications */
	y = (y * ifnt->hgt) + ifnt->asc + iwin->oy;


	/*** Decide where to place the string, horizontally ***/

	/* Line up with x at left edge of column 'x' */
	x = (x * ifnt->wid) + iwin->ox;


	/*** Actually draw 'str' onto the infowin ***/

	/* Be sure the correct font is ready */
	XSetFont(Metadpy->dpy, iclr->gc, ifnt->info->fid);


	/*** Handle the fake mono we can enforce on fonts ***/

	/* Monotize the font */
	if (ifnt->mono)
	{
		/* Do each character */
		for (int i = 0; i < len; ++i)
		{
			/* Note that the Infoclr is set up to contain the Infofnt */
			XDrawImageString(Metadpy->dpy, iwin->win, iclr->gc,
					 x + i * ifnt->wid + ifnt->off, y, str + i, 1);
		}
	}

	/* Assume monoospaced font */
	else
	{
		/* Note that the Infoclr is set up to contain the Infofnt */
		XDrawImageString(Metadpy->dpy, iwin->win, iclr->gc,
		                 x, y, str, len);
	}
}


/*
 * Painting where text would be
 */
static void Infofnt_text_non(infowin *iwin, infoclr *iclr, infofnt *ifnt, int x, int y, const char *str, int len)
{
	/*** Find the width ***/

	/* Negative length is a flag to count the characters in str */
	if (len < 0) len = strlen(str);

	/* The total width will be 'len' chars * standard width */
	int const w = len * ifnt->wid;


	/*** Find the X dimensions ***/

	/* Line up with x at left edge of column 'x' */
	x = x * ifnt->wid + iwin->ox;


	/*** Find other dimensions ***/

	/* Simply do 'Infofnt->hgt' (a single row) high */
	int const h = ifnt->hgt;

	/* Simply do "at top" in row 'y' */
	y = y * h + iwin->oy;


	/*** Actually 'paint' the area ***/

	/* Just do a Fill Rectangle */
	XFillRectangle(Metadpy->dpy, iwin->win, iclr->gc, x, y, w, h);
}



/*************************************************************************/


/*
 * Angband specific code follows... (ANGBAND)
 */


/*
 * Hack -- cursor color
 */
static infoclr *cursor_clr;

/*
 * Actual color table
 */
static infoclr *clr[256];

/*
 * Color info (unused, red, green, blue).
 */
static byte color_table[256][4];

/*
 * Forward declare
 */
typedef struct term_data term_data;

/*
 * A structure for each "term"
 */
struct term_data
{
	term *term_ptr;

	infofnt *fnt;

	infowin *win;


};


/*
 * The number of term data structures
 */
#define MAX_TERM_DATA 8

/*
 * The array of term data structures
 */
static term_data data[MAX_TERM_DATA];

/*
 * Process a keypress event
 *
 * Also appears in "main-xaw.c".
 */
static void react_keypress(XKeyEvent *xev)
{
	int i, n, mc, ms, mo, mx;

	unsigned int ks1;

	XKeyEvent *ev = (XKeyEvent*)(xev);

	KeySym ks;

	char buf[128];
	char msg[128];


	/* Check for "normal" keypresses */
	n = XLookupString(ev, buf, 125, &ks, NULL);

	/* Terminate */
	buf[n] = '\0';


	/* Hack -- Ignore "modifier keys" */
	if (IsModifierKey(ks)) return;


	/* Hack -- convert into an unsigned int */
	ks1 = (unsigned int)(ks);

	/* Extract four "modifier flags" */
	mc = (ev->state & ControlMask) ? true : false;
	ms = (ev->state & ShiftMask) ? true : false;
	mo = (ev->state & Mod1Mask) ? true : false;
	mx = (ev->state & Mod2Mask) ? true : false;


	/* Normal keys with no modifiers */
	if (n && !mo && !mx && !IsSpecialKey(ks))
	{
		/* Enqueue the normal key(s) */
		for (i = 0; buf[i]; i++) Term_keypress(buf[i]);

		/* All done */
		return;
	}


	/* Handle a few standard keys (bypass modifiers) XXX XXX XXX */
	switch (ks1)
	{
	case XK_Escape:
		{
			Term_keypress(ESCAPE);
			return;
		}

	case XK_Return:
		{
			Term_keypress('\r');
			return;
		}

	case XK_Tab:
		{
			Term_keypress('\t');
			return;
		}

	case XK_Delete:
	case XK_BackSpace:
		{
			Term_keypress('\010');
			return;
		}
	}


	/* Hack -- Use the KeySym */
	if (ks)
	{
		sprintf(msg, "%c%s%s%s%s_%lX%c", 31,
		        mc ? "N" : "", ms ? "S" : "",
		        mo ? "O" : "", mx ? "M" : "",
		        (unsigned long)(ks), 13);
	}

	/* Hack -- Use the Keycode */
	else
	{
		sprintf(msg, "%c%s%s%s%sK_%X%c", 31,
		        mc ? "N" : "", ms ? "S" : "",
		        mo ? "O" : "", mx ? "M" : "",
		        ev->keycode, 13);
	}

	/* Enqueue the "macro trigger" string */
	for (i = 0; msg[i]; i++) Term_keypress(msg[i]);


	/* Hack -- auto-define macros as needed */
	if (n && (macro_find_exact(msg) < 0))
	{
		/* Create a macro */
		macro_add(msg, buf);
	}
}

/*
 * Process events
 */
static errr CheckEvent(term_data *old_td, bool wait)
{
	XEvent xev_body, *xev = &xev_body;

	term_data *td = NULL;
	infowin *iwin = NULL;

	int i;


	/* Do not wait unless requested */
	if (!wait && !XPending(Metadpy->dpy)) return (1);

	/* Load the Event */
	XNextEvent(Metadpy->dpy, xev);


	/* Notice new keymaps */
	if (xev->type == MappingNotify)
	{
		XRefreshKeyboardMapping(&xev->xmapping);
		return 0;
	}


	/* Scan the windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		if (xev->xany.window == data[i].win->win)
		{
			td = &data[i];
			iwin = td->win;
			break;
		}
	}

	/* Unknown window */
	if (!td || !iwin) return (0);


	/* Hack -- activate the Term */
	Term_activate(td->term_ptr);

	/* Switch on the Type */
	switch (xev->type)
	{

	case KeyPress:
		{
			/* Hack -- use "old" term */
			Term_activate(old_td->term_ptr);

			/* Process the key */
			react_keypress(&(xev->xkey));

			break;
		}

	case Expose:
		{
			/* Ignore "extra" exposes */
			if (xev->xexpose.count) break;

			/* Clear the window */
			Infowin_wipe(iwin);

			/* Redraw */
			Term_redraw();

			break;
		}

	case MapNotify:
		{
			iwin->mapped = 1;
			Term_mapped();
			break;
		}

	case UnmapNotify:
		{
			iwin->mapped = 0;
			Term_unmapped();
			break;
		}

		/* Move and/or Resize */
	case ConfigureNotify:
		{
			int cols, rows;

			int ox = iwin->ox;
			int oy = iwin->oy;

			/* Save the new Window Parms */
			iwin->x = xev->xconfigure.x;
			iwin->y = xev->xconfigure.y;
			iwin->w = xev->xconfigure.width;
			iwin->h = xev->xconfigure.height;

			/* Determine "proper" number of rows/cols */
			cols = ((iwin->w - (ox + ox)) / td->fnt->wid);
			rows = ((iwin->h - (oy + oy)) / td->fnt->hgt);

			/* Hack -- minimal size */
			if (td == &data[0])
			{
				if (cols < 80) cols = 80;
				if (rows < 24) rows = 24;
			}

			else
			{
				if (cols < 1) cols = 1;
				if (rows < 1) rows = 1;
			}

			/* Paranoia */
			if (cols > 255) cols = 255;
			if (rows > 255) rows = 255;

			/* Resize the Term (if needed) */
			Term_resize(cols, rows);
			break;
		}
	}


	/* Hack -- Activate the old term */
	Term_activate(old_td->term_ptr);

	/* Success */
	return (0);
}


/**
 * UserInterace for X11
 */
class X11Frontend final : public Frontend {

private:
	term_data *m_term_data;

public:
	explicit X11Frontend(term_data *term_data)
		: m_term_data(term_data)
	{
	}

	void init() final
	{
		// No action necessary
	}

	bool soft_cursor() const final
	{
		return true;
	}

	bool icky_corner() const final
	{
		return false;
	}

	void nuke() final
	{
		// No action necessary
	}

	void process_event(bool wait) final
	{
		CheckEvent(m_term_data, wait);
	}

	void flush_events() final
	{
		while (!CheckEvent(m_term_data, false))
		{
			// Keep flushing
		}
	}

	void clear() final
	{
		Infowin_wipe(m_term_data->win);
	}

	void flush_output() final
	{
		XFlush(Metadpy->dpy);
	}

	void noise() final
	{
		XBell(Metadpy->dpy, 100);
	}

	void process_queued_events() final
	{
		CheckEvent(m_term_data, false);
	}

	void react() final
	{
		if (Metadpy->color)
		{
			/* Check the colors */
			for (int i = 0; i < 256; i++)
			{
				if ((color_table[i][0] != angband_color_table[i][0]) ||
						(color_table[i][1] != angband_color_table[i][1]) ||
						(color_table[i][2] != angband_color_table[i][2]) ||
						(color_table[i][3] != angband_color_table[i][3]))
				{
					Pixell pixel;

					/* Save new values */
					color_table[i][0] = angband_color_table[i][0];
					color_table[i][1] = angband_color_table[i][1];
					color_table[i][2] = angband_color_table[i][2];
					color_table[i][3] = angband_color_table[i][3];

					/* Create pixel */
					pixel = create_pixel(Metadpy->dpy,
							     color_table[i][1],
							     color_table[i][2],
							     color_table[i][3]);

					/* Change the foreground */
					Infoclr_change_fg(clr[i], pixel);
				}
			}
		}
	}

	void rename_main_window(std::string_view sv) final
	{
		Infowin_set_name(m_term_data->win, sv);
	}

	void draw_cursor(int x, int y) final
	{
		Infofnt_text_non(m_term_data->win, cursor_clr, m_term_data->fnt, x, y, " ", 1);
	}

	void draw_text(int x, int y, int n, byte a, const char *s) final
	{
		Infofnt_text_std(m_term_data->win, clr[a], m_term_data->fnt, x, y, s, n);
	}
};


/*
 * Initialize a term_data
 */
static term *term_data_init(term_data *td, int i)
{
	const char *name = angband_term_name[i];

	const char *font;

	int x = 0;
	int y = 0;

	int cols = 80;
	int rows = 24;

	int ox = 1;
	int oy = 1;

	int wid, hgt, num;

	char buf[80];

	const char *str;

	int val;

	XClassHint *ch;

	char res_name[20];
	char res_class[20];

	XSizeHints *sh;


	/* Window specific font name */
	sprintf(buf, "ANGBAND_X11_FONT_%d", i);

	/* Check environment for that font */
	font = getenv(buf);

	/* Check environment for "base" font */
	if (!font) font = getenv("ANGBAND_X11_FONT");

	/* No environment variables, use default font */
	if (!font)
	{
		switch (i)
		{
		case 0:
			{
				font = DEFAULT_X11_FONT;
			}
			break;
		case 1:
			{
				font = DEFAULT_X11_FONT;
			}
			break;
		case 2:
			{
				font = DEFAULT_X11_FONT;
			}
			break;
		case 3:
			{
				font = DEFAULT_X11_FONT;
			}
			break;
		case 4:
			{
				font = DEFAULT_X11_FONT;
			}
			break;
		case 5:
			{
				font = DEFAULT_X11_FONT;
			}
			break;
		case 6:
			{
				font = DEFAULT_X11_FONT;
			}
			break;
		case 7:
			{
				font = DEFAULT_X11_FONT;
			}
			break;
		default:
			{
				font = DEFAULT_X11_FONT;
			}
		}
	}

	/* Window specific location (x) */
	sprintf(buf, "ANGBAND_X11_AT_X_%d", i);
	str = getenv(buf);
	x = (str != NULL) ? atoi(str) : -1;

	/* Window specific location (y) */
	sprintf(buf, "ANGBAND_X11_AT_Y_%d", i);
	str = getenv(buf);
	y = (str != NULL) ? atoi(str) : -1;


	/* Window specific cols */
	sprintf(buf, "ANGBAND_X11_COLS_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) cols = val;

	/* Window specific rows */
	sprintf(buf, "ANGBAND_X11_ROWS_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) rows = val;


	/* Window specific inner border offset (ox) */
	sprintf(buf, "ANGBAND_X11_IBOX_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) ox = val;

	/* Window specific inner border offset (oy) */
	sprintf(buf, "ANGBAND_X11_IBOY_%d", i);
	str = getenv(buf);
	val = (str != NULL) ? atoi(str) : -1;
	if (val > 0) oy = val;


	/* Prepare the standard font */
	td->fnt = (infofnt *) calloc(1, sizeof(struct infofnt));
	if (td->fnt == NULL)
	{
		abort();
	}
	Infofnt_init_data(td->fnt, font);

	/* Hack -- key buffer size */
	num = (i == 0 ? 1024 : 16);

	/* Assume full size windows */
	wid = cols * td->fnt->wid + (ox + ox);
	hgt = rows * td->fnt->hgt + (oy + oy);

	/* Create a top-window */
	td->win = (infowin *) calloc(1, sizeof(struct infowin));
	if (td->win == NULL)
	{
		abort();
	}

	Infowin_init_data(td->win, None, x, y, wid, hgt, 0,
	                 Metadpy->fg, Metadpy->bg);

	/* Ask for certain events */
	Infowin_set_mask(td->win,
		ExposureMask | StructureNotifyMask | KeyPressMask |
		PointerMotionMask | ButtonPressMask | ButtonReleaseMask);

	/* Set the window name */
	Infowin_set_name(td->win, name);

	/* Save the inner border */
	td->win->ox = ox;
	td->win->oy = oy;

	/* Make Class Hints */
	ch = XAllocClassHint();

	if (ch == NULL) quit("XAllocClassHint failed");

	strcpy(res_name, name);
	res_name[0] = FORCELOWER(res_name[0]);
	ch->res_name = res_name;

	strcpy(res_class, "Angband");
	ch->res_class = res_class;

	XSetClassHint(Metadpy->dpy, td->win->win, ch);

	/* Make Size Hints */
	sh = XAllocSizeHints();

	/* Oops */
	if (sh == NULL) quit("XAllocSizeHints failed");

	/* Fixed window size */
	if (i == 0)
	{
		/* Main window: 80x24 -- 255x255 */
		sh->flags = PMinSize | PMaxSize;
		sh->min_width = 80 * td->fnt->wid + (ox + ox);
		sh->min_height = 24 * td->fnt->hgt + (oy + oy);
		sh->max_width = 255 * td->fnt->wid + (ox + ox);
		sh->max_height = 255 * td->fnt->hgt + (oy + oy);
	}

	/* Variable window size */
	else
	{
		/* Subwindows: 1x1 -- 255x255 */
		sh->flags = PMinSize | PMaxSize;
		sh->min_width = td->fnt->wid + (ox + ox);
		sh->min_height = td->fnt->hgt + (oy + oy);
		sh->max_width = 255 * td->fnt->wid + (ox + ox);
		sh->max_height = 255 * td->fnt->hgt + (oy + oy);
	}

	/* Resize increment */
	sh->flags |= PResizeInc;
	sh->width_inc = td->fnt->wid;
	sh->height_inc = td->fnt->hgt;

	/* Base window size */
	sh->flags |= PBaseSize;
	sh->base_width = (ox + ox);
	sh->base_height = (oy + oy);

	/* Use the size hints */
	XSetWMNormalHints(Metadpy->dpy, td->win->win, sh);

	/* Map the window */
	Infowin_map(td->win);


	/* Move the window to requested location */
	if ((x >= 0) && (y >= 0))
	{
		Infowin_impell(td->win, x, y);
	}

	/* Initialize the term */
	td->term_ptr = term_init(cols, rows, num, std::make_shared<X11Frontend>(td));

	/* Activate (important) */
	Term_activate(td->term_ptr);

	/* Success */
	return td->term_ptr;
}


/*
 * Initialization function for an "X11" module to Angband
 */
errr init_x11(int argc, char *argv[])
{
	int i;

	const char *dpy_name = "";

	int num_term = 1;



	/* Parse args */
	for (i = 1; i < argc; i++)
	{
		if (starts_with(argv[i], "-d"))
		{
			dpy_name = &argv[i][2];
			continue;
		}


		if (starts_with(argv[i], "-n"))
		{
			num_term = atoi(&argv[i][2]);
			if (num_term > MAX_TERM_DATA) num_term = MAX_TERM_DATA;
			else if (num_term < 1) num_term = 1;
			continue;
		}

		fprintf(stderr, "Ignoring option: %s", argv[i]);
	}


	/* Init the Metadpy if possible */
	if (Metadpy_new(dpy_name)) return ( -1);


	/* Prepare cursor color */
	cursor_clr = (infoclr *) calloc(1, sizeof(struct infoclr));
	if (cursor_clr == NULL)
	{
		abort();
	}
	Infoclr_init_data(cursor_clr, Metadpy->fg, Metadpy->bg, Infoclr_Opcode_XOR);


	/* Prepare normal colors */
	for (i = 0; i < 256; ++i)
	{
		Pixell pixel;

		clr[i] = (infoclr *) calloc(1, sizeof(struct infoclr));
		if (clr[i] == NULL)
		{
			abort();
		}

		/* Acquire Angband colors */
		color_table[i][0] = angband_color_table[i][0];
		color_table[i][1] = angband_color_table[i][1];
		color_table[i][2] = angband_color_table[i][2];
		color_table[i][3] = angband_color_table[i][3];

		/* Default to monochrome */
		pixel = ((i == 0) ? Metadpy->bg : Metadpy->fg);

		/* Handle color */
		if (Metadpy->color)
		{
			/* Create pixel */
			pixel = create_pixel(Metadpy->dpy,
			                     color_table[i][1],
			                     color_table[i][2],
			                     color_table[i][3]);
		}

		/* Initialize the color */
		Infoclr_init_data(clr[i], pixel, Metadpy->bg, Infoclr_Opcode_CPY);
	}


	/* Initialize the windows */
	for (i = 0; i < num_term; i++)
	{
		term_data *td = &data[i];

		/* Initialize the term_data */
		term *t = term_data_init(td, i);

		/* Save global entry */
		angband_term[i] = t;
	}

	/* Raise the "Angband" window */
	Infowin_raise(data[0].win);

	/* Activate the "Angband" window screen */
	Term_activate(data[0].term_ptr);




	/* Success */
	return (0);
}

int main(int argc, char *argv[])
{
	return main_real(
		argc,
		argv,
		"x11",
		init_x11,
		"  -- -n#             Number of terms to use\n"
		"  -- -d<name>        Display to use\n");
}

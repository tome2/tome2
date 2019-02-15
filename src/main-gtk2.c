/* File: main-gtk.c */

/*
 * Copyright (c) 2000-2001 Robert Ruehlmann,
 * Steven Fuerst, Uwe Siems, "pelpel", et al.
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

/*
 * Robert Ruehlmann wrote the original Gtk port. Since an initial work is
 * much harder than enhancements, his effort worth more credits than
 * others.
 *
 * Steven Fuerst implemented colour-depth independent X server support,
 * graphics, resizing and big screen support for ZAngband as well as
 * fast image rescaling that is included here.
 *
 * Uwe Siems wrote smooth tiles rescaling code (on by default).
 * Try this with 8x8 tiles. They *will* look different.
 *
 * "pelpel" wrote another colour-depth independent X support
 * using GdkRGB, added several hooks and callbacks for various
 * reasons, wrote no-backing store mode (off by default),
 * added GtkItemFactory based menu system, introduced
 * USE_GRAPHICS code bloat (^ ^;), added comments (I have
 * a strange habit of writing comments while I code...)
 * and reorganised the file a bit.
 */

#include "files.h"
#include "main.h"
#include "util.h"
#include "variable.h"


/* Force ANSI standard */
/* #define __STRICT_ANSI__ */

/* No GCC-specific includes */
/* #undef __GNUC__ */

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <assert.h>


/*
 * Number of pixels inserted between the menu bar and the main screen
 */
#define NO_PADDING 0


/*
 * Largest possible number of terminal windows supported by the game
 */
#define MAX_TERM_DATA 8


/*
 * Extra data to associate with each "window"
 *
 * Each "window" is represented by a "term_data" structure, which
 * contains a "term" structure, which contains a pointer (t->data)
 * back to the term_data structure.
 */



/*
 * This structure holds everything you need to manipulate terminals
 */
typedef struct term_data term_data;

struct term_data
{
	term *term_ptr;

	GtkWidget *window;
	GtkWidget *drawing_area;
	GdkPixmap *backing_store;
	GdkFont *font;
	GdkGC *gc;

	bool_ shown;
	byte last_attr;

	int font_wid;
	int font_hgt;

	int rows;
	int cols;


	char *name;
};


/*
 * Where to draw when we call Gdk drawing primitives
 */
# define TERM_DATA_DRAWABLE(td) \
((td)->backing_store ? (td)->backing_store : (td)->drawing_area->window)

# define TERM_DATA_REFRESH(td, x, y, wid, hgt) \
if ((td)->backing_store) gdk_draw_pixmap( \
(td)->drawing_area->window, \
(td)->gc, \
(td)->backing_store, \
(x) * (td)->font_wid, \
(y) * (td)->font_hgt, \
(x) * (td)->font_wid, \
(y) * (td)->font_hgt, \
(wid) * (td)->font_wid, \
(hgt) * (td)->font_hgt)


/*
 * An array of "term_data" structures, one for each "sub-window"
 */
static term_data data[MAX_TERM_DATA];

/*
 * Number of active terms
 */
static int num_term = 1;


/*
 * RGB values of the sixteen Angband colours
 */
static guint32 angband_colours[16];


/*
 * Set to TRUE when a game is in progress
 */
static bool_ game_in_progress = FALSE;


/*
 * This is in some cases used for double buffering as well as
 * a backing store, speeding things up under client-server
 * configurations, while turning this off *might* work better
 * with the MIT Shm extention which is usually active if you run
 * Angband locally, because it reduces amount of memory-to-memory copy.
 */
static bool_ use_backing_store = TRUE;




/**** Vanilla compatibility functions ****/

/*
 * Look up some environment variables to find font name for each window.
 */
static const char *get_default_font(int term)
{
	char buf[64];
	const char *font_name;

	/* Window specific font name */
	strnfmt(buf, 64, "ANGBAND_X11_FONT_%s", angband_term_name[term]);

	/* Check environment for that font */
	font_name = getenv(buf);

	/* Window specific font name */
	strnfmt(buf, 64, "ANGBAND_X11_FONT_%d", term);

	/* Check environment for that font */
	if (!font_name) font_name = getenv(buf);

	/* Check environment for "base" font */
	if (!font_name) font_name = getenv("ANGBAND_X11_FONT");

	/* No environment variables, use default font */
	if (!font_name) font_name = DEFAULT_X11_FONT_SCREEN;

	return (font_name);
}


/*
 * New global flag to indicate if it's safe to save now
 */
#define can_save TRUE




/**** Low level routines - colours and graphics ****/


/*
 * Remeber RGB values for sixteen Angband colours, in a format
 * that is convinient for GdkRGB GC functions.
 *
 * XXX XXX Duplication of maid-x11.c is far from the Angband
 * ideal of code cleanliness, but the whole point of using GdkRGB
 * is to let it handle colour allocation which it does in a very
 * clever fashion. Ditto for the tile scaling code and the BMP loader
 * below.
 */
static void init_colours(void)
{
	int i;


	/* Process each colour */
	for (i = 0; i < 16; i++)
	{
		u32b red, green, blue;

		/* Retrieve RGB values from the game */
		red = angband_color_table[i][1];
		green = angband_color_table[i][2];
		blue = angband_color_table[i][3];

		/* Remember a GdkRGB value, that is 0xRRGGBB */
		angband_colours[i] = (red << 16) | (green << 8) | blue;
	}
}


/*
 * Set foreground colour of window td to attr, only when it is necessary
 */
static void term_data_set_fg(term_data *td, byte attr)
{
	/* We can use the current gc */
	if (td->last_attr == attr) return;

	/* Activate the colour */
	gdk_rgb_gc_set_foreground(td->gc, angband_colours[attr]);

	/* Remember it */
	td->last_attr = attr;
}






/**** Term package support routines ****/


/*
 * Free data used by a term
 */
static void Term_nuke_gtk(void *data)
{
	term_data *td = (term_data *) data;

	/* Free name */
	if (td->name) free(td->name);

	/* Forget it */
	td->name = NULL;

	/* Free font */
	if (td->font) gdk_font_unref(td->font);

	/* Forget it */
	td->font = NULL;

	/* Free backing store */
	if (td->backing_store) gdk_pixmap_unref(td->backing_store);

	/* Forget it too */
	td->backing_store = NULL;

}


/*
 * Erase the whole term.
 */
static void Term_clear_gtk(term_data *td)
{
	/* Don't draw to hidden windows */
	if (!td->shown)
	{
		return;
	}

	/* Paranoia */
	g_assert(td->drawing_area->window != 0);

	/* Clear the area */
	gdk_draw_rectangle(
	        TERM_DATA_DRAWABLE(td),
	        td->drawing_area->style->black_gc,
	        1,
	        0,
	        0,
	        td->cols * td->font_wid,
	        td->rows * td->font_hgt);

	/* Copy image from backing store if present */
	TERM_DATA_REFRESH(td, 0, 0, td->cols, td->rows);
}


/*
 * Erase some characters.
 */
static errr Term_wipe_gtk(term_data *td, int x, int y, int n)
{
	/* Don't draw to hidden windows */
	if (!td->shown) return (0);

	/* Paranoia */
	g_assert(td->drawing_area->window != 0);

	/* Fill the area with the background colour */
	gdk_draw_rectangle(
	        TERM_DATA_DRAWABLE(td),
	        td->drawing_area->style->black_gc,
	        TRUE,
	        x * td->font_wid,
	        y * td->font_hgt,
	        n * td->font_wid,
	        td->font_hgt);

	/* Copy image from backing store if present */
	TERM_DATA_REFRESH(td, x, y, n, 1);

	/* Success */
	return (0);
}


/*
 * Draw some textual characters.
 */
static void Term_text_gtk(void *data, int x, int y, int n, byte a, const char *s)
{
	term_data *td = (term_data *) data;

	/* Don't draw to hidden windows */
	if (!td->shown)
	{
		return;
	}

	/* Paranoia */
	g_assert(td->drawing_area->window != 0);

	/* Set foreground colour */
	term_data_set_fg(td, a);

	/* Clear the line */
	Term_wipe_gtk(td, x, y, n);

	/* Draw the text to the window */
	gdk_draw_text(
	        TERM_DATA_DRAWABLE(td),
	        td->font,
	        td->gc,
	        x * td->font_wid,
	        td->font->ascent + y * td->font_hgt,
	        s,
	        n);

	/* Copy image from backing store if present */
	TERM_DATA_REFRESH(td, x, y, n, 1);
}


/*
 * Draw software cursor at (x, y)
 */
static void Term_curs_gtk(void *data, int x, int y)
{
	term_data *td = (term_data *) data;
	int cells = 1;


	/* Don't draw to hidden windows */
	if (!td->shown)
	{
		return;
	}

	/* Paranoia */
	g_assert(td->drawing_area->window != 0);

	/* Set foreground colour */
	term_data_set_fg(td, TERM_YELLOW);

	/* Draw the software cursor */
	gdk_draw_rectangle(
	        TERM_DATA_DRAWABLE(td),
	        td->gc,
	        FALSE,
	        x * td->font_wid,
	        y * td->font_hgt,
	        td->font_wid * cells - 1,
	        td->font_hgt - 1);

	/* Copy image from backing store if present */
	TERM_DATA_REFRESH(td, x, y, cells, 1);
}




/*
 * Process an event, if there's none block when wait is set true,
 * return immediately otherwise.
 */
static void CheckEvent(bool_ wait)
{
	/* Process an event */
	(void)gtk_main_iteration_do(wait);
}


/*
 * Process all pending events (without blocking)
 */
static void DrainEvents(void)
{
	while (gtk_events_pending())
		gtk_main_iteration();
}


/*
 * Handle a "special request"
 */
static void Term_xtra_gtk(void *term_data_ctx, int n, int v)
{
	term_data *td = (term_data *) term_data_ctx;

	/* Handle a subset of the legal requests */
	switch (n)
	{
		/* Make a noise */
	case TERM_XTRA_NOISE:
		{
			gdk_beep();
			return;
		}

		/* Flush the output */
	case TERM_XTRA_FRESH:
		{
			gdk_flush();
			return;
		}

		/* Process random events */
	case TERM_XTRA_BORED:
		{
			CheckEvent(FALSE);
			return;
		}

		/* Process Events */
	case TERM_XTRA_EVENT:
		{
			CheckEvent(v);
			return;
		}

		/* Flush the events */
	case TERM_XTRA_FLUSH:
		{
			DrainEvents();
			return;
		}

		/* Handle change in the "level" */
	case TERM_XTRA_LEVEL:
		return;

		/* Clear the screen */
	case TERM_XTRA_CLEAR:
		Term_clear_gtk(td);
		return;

		/* Rename main window */
	case TERM_XTRA_RENAME_MAIN_WIN:
		gtk_window_set_title(GTK_WINDOW(data[0].window), angband_term_name[0]);
		return;

		/* React to changes */
	case TERM_XTRA_REACT:
		{
			init_colours();
			return;
		}
	}
}




/**** Event handlers ****/


/*
 * Operation overkill
 * Verify term size info - just because the other windowing ports have this
 */
static void term_data_check_size(term_data *td)
{
	/* Enforce minimum window size */
	if (td == &data[0])
	{
		if (td->cols < 80) td->cols = 80;
		if (td->rows < 24) td->rows = 24;
	}
	else
	{
		if (td->cols < 1) td->cols = 1;
		if (td->rows < 1) td->rows = 1;
	}

	/* Paranoia - Enforce maximum size allowed by the term package */
	if (td->cols > 255) td->cols = 255;
	if (td->rows > 255) td->rows = 255;
}


/*
 * Enforce these size constraints within Gtk/Gdk
 * These increments are nice, because you can see numbers of rows/cols
 * while you resize a term.
 */
static void term_data_set_geometry_hints(term_data *td)
{
	GdkGeometry geometry;

	/* Resizing is character size oriented */
	geometry.width_inc = td->font_wid;
	geometry.height_inc = td->font_hgt;

	/* Enforce minimum size - the main window */
	if (td == &data[0])
	{
		geometry.min_width = 80 * td->font_wid;
		geometry.min_height = 24 * td->font_hgt;
	}

	/* Subwindows can be much smaller */
	else
	{
		geometry.min_width = 1 * td->font_wid;
		geometry.min_height = 1 * td->font_hgt;
	}

	/* Enforce term package's hard limit */
	geometry.max_width = 255 * td->font_wid;
	geometry.max_height = 255 * td->font_hgt;

	/* This affects geometry display while we resize a term */
	geometry.base_width = 0;
	geometry.base_height = 0;

	/* Give the window a new set of resizing hints */
	gtk_window_set_geometry_hints(GTK_WINDOW(td->window),
	                              td->drawing_area, &geometry,
	                              GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE
	                              | GDK_HINT_BASE_SIZE | GDK_HINT_RESIZE_INC);
}


/*
 * (Re)allocate a backing store for the window
 */
static void term_data_set_backing_store(term_data *td)
{
	/* Paranoia */
	if (!GTK_WIDGET_REALIZED(td->drawing_area)) return;

	/* Free old one if we cannot use it any longer */
	if (td->backing_store)
	{
		int wid, hgt;

		/* Retrive the size of the old backing store */
		gdk_window_get_size(td->backing_store, &wid, &hgt);

		/* Continue using it if it's the same with desired size */
		if (use_backing_store &&
		                (td->cols * td->font_wid == wid) &&
		                (td->rows * td->font_hgt == hgt)) return;

		/* Free it */
		gdk_pixmap_unref(td->backing_store);

		/* Forget the pointer */
		td->backing_store = NULL;
	}

	/* See user preference */
	if (use_backing_store)
	{
		/* Allocate new backing store */
		td->backing_store = gdk_pixmap_new(
		                            td->drawing_area->window,
		                            td->cols * td->font_wid,
		                            td->rows * td->font_hgt,
		                            -1);

		/* Oops - but we can do without it */
		g_return_if_fail(td->backing_store != NULL);

		/* Clear the backing store */
		gdk_draw_rectangle(
		        td->backing_store,
		        td->drawing_area->style->black_gc,
		        TRUE,
		        0,
		        0,
		        td->cols * td->font_wid,
		        td->rows * td->font_hgt);
	}
}


/*
 * Save game only when it's safe to do so
 */
static void save_game_gtk(void)
{
	/* We have nothing to save, yet */
	if (!game_in_progress || !character_generated) return;

	/* It isn't safe to save game now */
	if (!inkey_flag || !can_save)
	{
		plog("You may not save right now.");
		return;
	}

	/* Hack -- Forget messages */
	msg_flag = FALSE;

	/* Save the game */
	do_cmd_save_game();
}


/*
 * Display message in a modal dialog
 */
static void gtk_message(const char *msg)
{
	GtkWidget *dialog, *label, *ok_button;

	/* Create the widgets */
	dialog = gtk_dialog_new();
	g_assert(dialog != NULL);

	label = gtk_label_new(msg);
	g_assert(label != NULL);

	ok_button = gtk_button_new_with_label("OK");
	g_assert(ok_button != NULL);

	/* Ensure that the dialogue box is destroyed when OK is clicked */
	gtk_signal_connect_object(
	        GTK_OBJECT(ok_button),
	        "clicked",
	        GTK_SIGNAL_FUNC(gtk_widget_destroy),
	        (gpointer)dialog);
	gtk_container_add(
	        GTK_CONTAINER(GTK_DIALOG(dialog)->action_area),
	        ok_button);

	/* Add the label, and show the dialog */
	gtk_container_add(
	        GTK_CONTAINER(GTK_DIALOG(dialog)->vbox),
	        label);

	/* And make it modal */
	gtk_window_set_modal(GTK_WINDOW(dialog), TRUE);

	/* Show the dialog */
	gtk_widget_show_all(dialog);
}


/*
 * Hook to tell the user something important
 */
static void hook_plog(const char *str)
{
	/* Warning message */
	gtk_message(str);
}


/*
 * Process File-Quit menu command
 */
static void quit_event_handler(
        gpointer user_data,
		guint user_action,
        GtkWidget *was_clicked)
{
	/* Save current game */
	save_game_gtk();

	/* It's done */
	quit(NULL);
}


/*
 * Process File-Save menu command
 */
static void save_event_handler(
        gpointer user_data,
		guint user_action,
        GtkWidget *was_clicked)
{
	/* Save current game */
	save_game_gtk();
}


/*
 * Handle destruction of the Angband window
 */
static void destroy_main_event_handler(
        GtkButton *was_clicked,
        gpointer user_data)
{
	/* This allows for cheating, but... */
	quit(NULL);
}


/*
 * Handle destruction of Subwindows
 */
static void destroy_sub_event_handler(
        GtkWidget *window,
        gpointer user_data)
{
	/* Hide the window */
	gtk_widget_hide_all(window);
}


/*
 * Load fond specified by an XLFD fontname and
 * set up related term_data members
 */
static void load_font(term_data *td, const char *fontname)
{
	GdkFont *old = td->font;

	/* Load font */
	td->font = gdk_font_load(fontname);

	if (td->font)
	{
		/* Free the old font */
		if (old) gdk_font_unref(old);
	}
	else
	{
		/* Oops, but we can still use the old one */
		td->font = old;
	}

	/* Calculate the size of the font XXX */
	td->font_wid = gdk_char_width(td->font, '@');
	td->font_hgt = td->font->ascent + td->font->descent;

}


/*
 * Process Options-Font-* menu command
 */
static void change_font_event_handler(
        gpointer user_data,
		guint user_action,
        GtkWidget *widget)
{
	/* Not implemented */
}


/*
 * Process Terms-* menu command - hide/show terminal window
 */
static void term_event_handler(
		gpointer user_data,
        guint user_action,
        GtkWidget *widget)
{
	term_data *td = &data[user_action];

	/* We don't mess with the Angband window */
	if (td == &data[0]) return;

	/* It's shown */
	if (td->shown)
	{
		/* Hide the window */
		gtk_widget_hide_all(td->window);
	}

	/* It's hidden */
	else
	{
		/* Show the window */
		gtk_widget_show_all(td->window);
	}
}


/*
 * Toggles the boolean value of use_backing_store and
 * setup / remove backing store for each term
 */
static void change_backing_store_event_handler(
        gpointer user_data,
		guint user_action,
        GtkWidget *was_clicked)
{
	int i;

	/* Toggle the backing store mode */
	use_backing_store = !use_backing_store;

	/* Reset terms */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term_data_set_backing_store(&data[i]);
	}
}




/*
 * React to "delete" signal sent to Window widgets
 */
static gboolean delete_event_handler(
        GtkWidget *widget,
        GdkEvent *event,
        gpointer user_data)
{
	/* Save game if possible */
	save_game_gtk();

	/* Don't prevent closure */
	return (FALSE);
}


/*
 * Convert keypress events to ASCII codes and enqueue them
 * for game
 */
static gboolean keypress_event_handler(
        GtkWidget *widget,
        GdkEventKey *event,
        gpointer user_data)
{
	int i, mc, ms, mo, mx;

	char msg[128];

	/* Hack - do not do anything until the player picks from the menu */
	if (!game_in_progress) return (TRUE);

	/* Hack - Ignore parameters */
	(void) widget;
	(void) user_data;

	/* Extract four "modifier flags" */
	mc = (event->state & GDK_CONTROL_MASK) ? TRUE : FALSE;
	ms = (event->state & GDK_SHIFT_MASK) ? TRUE : FALSE;
	mo = (event->state & GDK_MOD1_MASK) ? TRUE : FALSE;
	mx = (event->state & GDK_MOD3_MASK) ? TRUE : FALSE;

	/*
	 * Hack XXX
	 * Parse shifted numeric (keypad) keys specially.
	 */
	if ((event->state == GDK_SHIFT_MASK)
	                && (event->keyval >= GDK_KP_0) && (event->keyval <= GDK_KP_9))
	{
		/* Build the macro trigger string */
		strnfmt(msg, 128, "%cS_%X%c", 31, event->keyval, 13);

		/* Enqueue the "macro trigger" string */
		for (i = 0; msg[i]; i++) Term_keypress(msg[i]);

		/* Hack -- auto-define macros as needed */
		if (event->length && (macro_find_exact(msg) < 0))
		{
			/* Create a macro */
			macro_add(msg, event->string);
		}

		return (TRUE);
	}

	/* Normal keys with no modifiers */
	if (event->length && !mo && !mx)
	{
		/* Enqueue the normal key(s) */
		for (i = 0; i < event->length; i++) Term_keypress(event->string[i]);

		/* All done */
		return (TRUE);
	}


	/* Handle a few standard keys (bypass modifiers) XXX XXX XXX */
	switch ((unsigned int) event->keyval)
	{
	case GDK_Escape:
		{
			Term_keypress(ESCAPE);
			return (TRUE);
		}

	case GDK_Return:
		{
			Term_keypress('\r');
			return (TRUE);
		}

	case GDK_Tab:
		{
			Term_keypress('\t');
			return (TRUE);
		}

	case GDK_Delete:
	case GDK_BackSpace:
		{
			Term_keypress('\010');
			return (TRUE);
		}

	case GDK_Shift_L:
	case GDK_Shift_R:
	case GDK_Control_L:
	case GDK_Control_R:
	case GDK_Caps_Lock:
	case GDK_Shift_Lock:
	case GDK_Meta_L:
	case GDK_Meta_R:
	case GDK_Alt_L:
	case GDK_Alt_R:
	case GDK_Super_L:
	case GDK_Super_R:
	case GDK_Hyper_L:
	case GDK_Hyper_R:
		{
			/* Hack - do nothing to control characters */
			return (TRUE);
		}
	}

	/* Build the macro trigger string */
	strnfmt(msg, 128, "%c%s%s%s%s_%X%c", 31,
	        mc ? "N" : "", ms ? "S" : "",
	        mo ? "O" : "", mx ? "M" : "",
	        event->keyval, 13);

	/* Enqueue the "macro trigger" string */
	for (i = 0; msg[i]; i++) Term_keypress(msg[i]);

	/* Hack -- auto-define macros as needed */
	if (event->length && (macro_find_exact(msg) < 0))
	{
		/* Create a macro */
		macro_add(msg, event->string);
	}

	return (TRUE);
}


/*
 * Widget customisation (for drawing area) - "realize" signal
 *
 * In this program, called when window containing the drawing
 * area is shown first time.
 */
static void realize_event_handler(
        GtkWidget *widget,
        gpointer user_data)
{
	term_data *td = (term_data *)user_data;

	/* Create graphic context */
	td->gc = gdk_gc_new(td->drawing_area->window);

	/* Set foreground and background colours - isn't bg used at all? */
	gdk_rgb_gc_set_background(td->gc, 0x000000);
	gdk_rgb_gc_set_foreground(td->gc, angband_colours[TERM_WHITE]);

	/* No last foreground colour, yet */
	td->last_attr = -1;

	/* Allocate the backing store */
	term_data_set_backing_store(td);

	/* Clear the window */
	gdk_draw_rectangle(
	        widget->window,
	        widget->style->black_gc,
	        TRUE,
	        0,
	        0,
	        td->cols * td->font_wid,
	        td->rows * td->font_hgt);
}


/*
 * Widget customisation (for drawing area) - "show" signal
 */
static void show_event_handler(
        GtkWidget *widget,
        gpointer user_data)
{
	term_data *td = (term_data *)user_data;

	/* Set the shown flag */
	td->shown = TRUE;
}


/*
 * Widget customisation (for drawing area) - "hide" signal
 */
static void hide_event_handler(
        GtkWidget *widget,
        gpointer user_data)
{
	term_data *td = (term_data *)user_data;

	/* Set the shown flag */
	td->shown = FALSE;
}


/*
 * Widget customisation (for drawing area)- handle size allocation requests
 */
static void size_allocate_event_handler(
        GtkWidget *widget,
        GtkAllocation *allocation,
        gpointer user_data)
{
	term_data *td = user_data;
	int old_rows, old_cols;
	term *old = Term;

	/* Paranoia */
	g_return_if_fail(widget != NULL);
	g_return_if_fail(allocation != NULL);
	g_return_if_fail(td != NULL);

	/* Remember old values */
	old_cols = td->cols;
	old_rows = td->rows;

	/* Update numbers of rows and columns */
	td->cols = (allocation->width + td->font_wid - 1) / td->font_wid;
	td->rows = (allocation->height + td->font_hgt - 1) / td->font_hgt;

	/* Overkill - Validate them */
	term_data_check_size(td);

	/* Adjust size request and set it */
	allocation->width = td->cols * td->font_wid;
	allocation->height = td->rows * td->font_hgt;
	widget->allocation = *allocation;

	/* Widget is realized, so we do some drawing works */
	if (GTK_WIDGET_REALIZED(widget))
	{
		/* Reallocate the backing store */
		term_data_set_backing_store(td);

		/* Actually handles resizing in Gtk */
		gdk_window_move_resize(
		        widget->window,
		        allocation->x,
		        allocation->y,
		        allocation->width,
		        allocation->height);

		/* And in the term package */
		Term_activate(td->term_ptr);

		/* Resize if necessary */
		if ((td->cols != old_cols) || (td->rows != old_rows))
		{
			Term_resize(td->cols, td->rows);
		}

		/* Redraw its content */
		Term_redraw();

		/* Refresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}
}


/*
 * Update exposed area in a window (for drawing area)
 */
static gboolean expose_event_handler(
        GtkWidget *widget,
        GdkEventExpose *event,
        gpointer user_data)
{
	term_data *td = user_data;

	term *old = Term;

#ifndef NO_REDRAW_SECTION

	int x1, x2, y1, y2;

#endif /* !NO_REDRAW_SECTION */


	/* Paranoia */
	if (td == NULL) return (TRUE);

	/* The window has a backing store */
	if (td->backing_store)
	{
		/* Simply restore the exposed area from the backing store */
		gdk_draw_pixmap(
		        td->drawing_area->window,
		        td->gc,
		        td->backing_store,
		        event->area.x,
		        event->area.y,
		        event->area.x,
		        event->area.y,
		        event->area.width,
		        event->area.height);
	}

	/* No backing store - use the game's code to redraw the area */
	else
	{

		/* Activate the relevant term */
		Term_activate(td->term_ptr);

# ifdef NO_REDRAW_SECTION

		/* K.I.S.S. version */

		/* Redraw */
		Term_redraw();

# else /* NO_REDRAW_SECTION */

		/*
		 * Complex version - The above is enough, but since we have
		 * Term_redraw_section... This might help if we had a graphics
		 * mode.
		 */

		/* Convert coordinate in pixels to character cells */
		x1 = event->area.x / td->font_wid;
		x2 = (event->area.x + event->area.width) / td->font_wid;
		y1 = event->area.y / td->font_hgt;
		y2 = (event->area.y + event->area.height) / td->font_hgt;

		/*
		 * No paranoia - boundary checking is done in
		 * Term_redraw_section
		 */

		/* Redraw the area */
		Term_redraw_section(x1, y1, x2, y2);

# endif  /* NO_REDRAW_SECTION */

		/* Refresh */
		Term_fresh();

		/* Restore */
		Term_activate(old);
	}

	/* We've processed the event ourselves */
	return (TRUE);
}




/**** Initialisation ****/

/*
 * Initialise a term_data struct
 */
static errr term_data_init(term_data *td, int i)
{
	char *p;

	td->cols = 80;
	td->rows = 24;

	/* Hooks */
	struct term_ui_hooks_t ui_hooks = {
		NULL /* init */,
		Term_nuke_gtk,
		Term_xtra_gtk,
		Term_curs_gtk,
		Term_text_gtk,
	};

	/* Initialize the term */
	td->term_ptr = term_init(td, td->cols, td->rows, 1024);
	term_init_soft_cursor(td->term_ptr);
	term_init_ui_hooks(td->term_ptr, ui_hooks);

	/* Store the name of the term */
	assert(angband_term_name[i] != NULL);
	td->name = strdup(angband_term_name[i]);

	/* Instance names should start with a lowercase letter XXX */
	for (p = (char *)td->name; *p; p++) *p = tolower(*p);

	/* Activate (important) */
	Term_activate(td->term_ptr);

	/* Success */
	return (0);
}


/*
 * Neater menu code with GtkItemFactory.
 *
 * Menu bar of the Angband window
 *
 * Entry format: Path, Accelerator, Callback, Callback arg, type
 * where type is one of:
 * <Item> - simple item, alias NULL
 * <Branch> - has submenu
 * <Separator> - as you read it
 * <CheckItem> - has a check mark
 * <ToggleItem> - is a toggle
 */
static GtkItemFactoryEntry main_menu_items[] =
{
	/* "File" menu */
	{ "/File", NULL,
	  NULL, 0, "<Branch>", NULL
	},
	{ "/File/Save", "<mod1>S",
	  save_event_handler, 0, NULL, NULL },
	{ "/File/Quit", "<mod1>Q",
	  quit_event_handler, 0, NULL, NULL },

	/* "Terms" menu */
	{ "/Terms", NULL,
	  NULL, 0, "<Branch>", NULL },
	/* XXX XXX XXX NULL's are replaced by the program */
	{ NULL, "<mod1>0",
	  term_event_handler, 0, "<CheckItem>", NULL },
	{ NULL, "<mod1>1",
	  term_event_handler, 1, "<CheckItem>", NULL },
	{ NULL, "<mod1>2",
	  term_event_handler, 2, "<CheckItem>", NULL },
	{ NULL, "<mod1>3",
	  term_event_handler, 3, "<CheckItem>", NULL },
	{ NULL, "<mod1>4",
	  term_event_handler, 4, "<CheckItem>", NULL },
	{ NULL, "<mod1>5",
	  term_event_handler, 5, "<CheckItem>", NULL },
	{ NULL, "<mod1>6",
	  term_event_handler, 6, "<CheckItem>", NULL },
	{ NULL, "<mod1>7",
	  term_event_handler, 7, "<CheckItem>", NULL },

	/* "Options" menu */
	{ "/Options", NULL,
	  NULL, 0, "<Branch>", NULL },

	/* "Font" submenu */
	{ "/Options/Font", NULL,
	  NULL, 0, "<Branch>", NULL },
	/* XXX XXX XXX Again, NULL's are filled by the program */
	{ NULL, NULL,
	  change_font_event_handler, 0, NULL, NULL },
	{ NULL, NULL,
	  change_font_event_handler, 1, NULL, NULL },
	{ NULL, NULL,
	  change_font_event_handler, 2, NULL, NULL },
	{ NULL, NULL,
	  change_font_event_handler, 3, NULL, NULL },
	{ NULL, NULL,
	  change_font_event_handler, 4, NULL, NULL },
	{ NULL, NULL,
	  change_font_event_handler, 5, NULL, NULL },
	{ NULL, NULL,
	  change_font_event_handler, 6, NULL, NULL },
	{ NULL, NULL,
	  change_font_event_handler, 7, NULL, NULL },


	/* "Misc" submenu */
	{ "/Options/Misc", NULL,
	  NULL, 0, "<Branch>", NULL },
	{ "/Options/Misc/Backing store", NULL,
	  change_backing_store_event_handler, 0, "<CheckItem>", NULL },
};


/*
 * XXX XXX Fill those NULL's in the menu definition with
 * angband_term_name[] strings
 */
static void setup_menu_paths(void)
{
	int i;
	int nmenu_items = sizeof(main_menu_items) / sizeof(main_menu_items[0]);
	GtkItemFactoryEntry *term_entry, *font_entry;
	char buf[64];

	/* Find the "Terms" menu */
	for (i = 0; i < nmenu_items; i++)
	{
		/* Skip NULLs */
		if (main_menu_items[i].path == NULL) continue;

		/* Find a match */
		if (streq(main_menu_items[i].path, "/Terms")) break;
	}
	g_assert(i < (nmenu_items - MAX_TERM_DATA));

	/* Remember the location */
	term_entry = &main_menu_items[i + 1];

	/* Find "Font" menu */
	for (i = 0; i < nmenu_items; i++)
	{
		/* Skip NULLs */
		if (main_menu_items[i].path == NULL) continue;

		/* Find a match */
		if (streq(main_menu_items[i].path, "/Options/Font")) break;
	}
	g_assert(i < (nmenu_items - MAX_TERM_DATA));

	/* Remember the location */
	font_entry = &main_menu_items[i + 1];

	/* For each terminal */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* XXX XXX Build the real path name to the entry */
		strnfmt(buf, 64, "/Terms/%s", angband_term_name[i]);

		/* XXX XXX Store it in the menu definition */
		term_entry[i].path = (gchar*) strdup(buf);

		/* XXX XXX Build the real path name to the entry */
		strnfmt(buf, 64, "/Options/Font/%s", angband_term_name[i]);

		/* XXX XXX Store it in the menu definition */
		font_entry[i].path = (gchar*) strdup(buf);
	}
}


/*
 * XXX XXX Free strings allocated by setup_menu_paths()
 */
static void free_menu_paths(void)
{
	int i;
	int nmenu_items = sizeof(main_menu_items) / sizeof(main_menu_items[0]);
	GtkItemFactoryEntry *term_entry, *font_entry;

	/* Find the "Terms" menu */
	for (i = 0; i < nmenu_items; i++)
	{
		/* Skip NULLs */
		if (main_menu_items[i].path == NULL) continue;

		/* Find a match */
		if (streq(main_menu_items[i].path, "/Terms")) break;
	}
	g_assert(i < (nmenu_items - MAX_TERM_DATA));

	/* Remember the location */
	term_entry = &main_menu_items[i + 1];

	/* Find "Font" menu */
	for (i = 0; i < nmenu_items; i++)
	{
		/* Skip NULLs */
		if (main_menu_items[i].path == NULL) continue;

		/* Find a match */
		if (streq(main_menu_items[i].path, "/Options/Font")) break;
	}
	g_assert(i < (nmenu_items - MAX_TERM_DATA));

	/* Remember the location */
	font_entry = &main_menu_items[i + 1];

	/* For each terminal */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* XXX XXX Free Term menu path */
		if (term_entry[i].path) free(term_entry[i].path);

		/* XXX XXX Free Font menu path */
		if (font_entry[i].path) free(font_entry[i].path);
	}
}


/*
 * Find widget corresponding to path name
 * return NULL on error
 */
static GtkWidget *get_widget_from_path(const char *path)
{
	GtkItemFactory *item_factory;
	GtkWidget *widget;

	/* Paranoia */
	if (path == NULL) return (NULL);

	/* Look up item factory */
	item_factory = gtk_item_factory_from_path(path);

	/* Oops */
	if (item_factory == NULL) return (NULL);

	/* Look up widget */
	widget = gtk_item_factory_get_widget(item_factory, path);

	/* Return result */
	return (widget);
}


/*
 * Enable/disable a menu item
 */
void enable_menu_item(const char *path, bool_ enabled)
{
	GtkWidget *widget;

	/* Access menu item widget */
	widget = get_widget_from_path(path);

	/* Paranoia */
	g_assert(widget != NULL);
	g_assert(GTK_IS_MENU_ITEM(widget));

	/*
	 * In Gtk's terminology, enabled is sensitive
	 * and disabled insensitive
	 */
	gtk_widget_set_sensitive(widget, enabled);
}


/*
 * Check/uncheck a menu item. The item should be of the GtkCheckMenuItem type
 */
void check_menu_item(const char *path, bool_ checked)
{
	GtkWidget *widget;

	/* Access menu item widget */
	widget = get_widget_from_path(path);

	/* Paranoia */
	g_assert(widget != NULL);
	g_assert(GTK_IS_CHECK_MENU_ITEM(widget));

	/*
	 * Put/remove check mark
	 *
	 * Mega-Hack -- The function supposed to be used here,
	 * gtk_check_menu_item_set_active(), emits an "activate" signal
	 * to the GtkMenuItem class of the widget, as if the menu item
	 * were selected by user, thereby causing bizarre behaviour.
	 * XXX XXX XXX
	 */
	GTK_CHECK_MENU_ITEM(widget)->active = checked;
}


/*
 * Update the "File" menu
 */
static void file_menu_update_handler(
        GtkWidget *widget,
        gpointer user_data)
{
	bool_ save_ok, quit_ok;

	/* Cave we save/quit now? */
	if (!character_generated || !game_in_progress)
	{
		save_ok = FALSE;
		quit_ok = TRUE;
	}
	else
	{
		if (inkey_flag && can_save) save_ok = quit_ok = TRUE;
		else save_ok = quit_ok = FALSE;
	}

	/* Enable / disable menu items according to those conditions */
	enable_menu_item("<Angband>/File/Save", save_ok);
	enable_menu_item("<Angband>/File/Quit", quit_ok);
}


/*
 * Update the "Terms" menu
 */
static void term_menu_update_handler(
        GtkWidget *widget,
        gpointer user_data)
{
	int i;
	char buf[64];

	/* For each term */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Build the path name */
		strnfmt(buf, 64, "<Angband>/Terms/%s", angband_term_name[i]);

		/* Update the check mark on the item */
		check_menu_item(buf, data[i].shown);
	}
}


/*
 * Update the "Font" submenu
 */
static void font_menu_update_handler(
        GtkWidget *widget,
        gpointer user_data)
{
	int i;
	char buf[64];

	/* For each term */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Build the path name */
		strnfmt(buf, 64, "<Angband>/Options/Font/%s", angband_term_name[i]);

		/* Enable selection if the term is shown */
		enable_menu_item(buf, data[i].shown);
	}
}


/*
 * Update the "Misc" submenu
 */
static void misc_menu_update_handler(
        GtkWidget *widget,
        gpointer user_data)
{
	/* Update an item */
	check_menu_item(
	        "<Angband>/Options/Misc/Backing store",
	        use_backing_store);
}




/*
 * Construct a menu hierarchy using GtkItemFactory, setting up
 * callbacks and accelerators along the way, and return
 * a GtkMenuBar widget.
 */
GtkWidget *get_main_menu(term_data *td)
{
	GtkItemFactory *item_factory;
	GtkAccelGroup *accel_group;
	gint nmenu_items = sizeof(main_menu_items) / sizeof(main_menu_items[0]);


	/* XXX XXX Setup path names in the "Terms" and "Font" menus */
	setup_menu_paths();

	/* Allocate an accelerator group */
	accel_group = gtk_accel_group_new();
	g_assert(accel_group != NULL);

	/* Initialise the item factory */
	item_factory = gtk_item_factory_new(
	                       GTK_TYPE_MENU_BAR,
	                       "<Angband>",
	                       accel_group);
	g_assert(item_factory != NULL);

	/* Generate the menu items */
	gtk_item_factory_create_items(
	        item_factory,
	        nmenu_items,
	        main_menu_items,
	        NULL);

	/* Attach the new accelerator group to the window */
	gtk_window_add_accel_group(
	        GTK_WINDOW(td->window),
	        accel_group);

	/* Return the actual menu bar created */
	return (gtk_item_factory_get_widget(item_factory, "<Angband>"));
}


/*
 * Install callbacks to update menus
 */
static void add_menu_update_callbacks()
{
	GtkWidget *widget;

	/* Access the "File" menu */
	widget = get_widget_from_path("<Angband>/File");

	/* Paranoia */
	g_assert(widget != NULL);
	g_assert(GTK_IS_MENU(widget));

	/* Assign callback */
	gtk_signal_connect(
	        GTK_OBJECT(widget),
	        "show",
	        GTK_SIGNAL_FUNC(file_menu_update_handler),
	        NULL);

	/* Access the "Terms" menu */
	widget = get_widget_from_path("<Angband>/Terms");

	/* Paranoia */
	g_assert(widget != NULL);
	g_assert(GTK_IS_MENU(widget));

	/* Assign callback */
	gtk_signal_connect(
	        GTK_OBJECT(widget),
	        "show",
	        GTK_SIGNAL_FUNC(term_menu_update_handler),
	        NULL);

	/* Access the "Font" menu */
	widget = get_widget_from_path("<Angband>/Options/Font");

	/* Paranoia */
	g_assert(widget != NULL);
	g_assert(GTK_IS_MENU(widget));

	/* Assign callback */
	gtk_signal_connect(
	        GTK_OBJECT(widget),
	        "show",
	        GTK_SIGNAL_FUNC(font_menu_update_handler),
	        NULL);

	/* Access the "Misc" menu */
	widget = get_widget_from_path("<Angband>/Options/Misc");

	/* Paranoia */
	g_assert(widget != NULL);
	g_assert(GTK_IS_MENU(widget));

	/* Assign callback */
	gtk_signal_connect(
	        GTK_OBJECT(widget),
	        "show",
	        GTK_SIGNAL_FUNC(misc_menu_update_handler),
	        NULL);

}


/*
 * Create Gtk widgets for a terminal window and set up callbacks
 */
static void init_gtk_window(term_data *td, int i)
{
	GtkWidget *menu_bar = NULL, *box;
	const char *font;

	bool_ main_window = (i == 0) ? TRUE : FALSE;


	/* Create window */
	td->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);

	/* Set title */
	gtk_window_set_title(GTK_WINDOW(td->window), td->name);


	/* Get default font for this term */
	font = get_default_font(i);

	/* Load font and initialise related term_data fields */
	load_font(td, font);


	/* Create drawing area */
	td->drawing_area = gtk_drawing_area_new();

	/* Set the size of the drawing area */
	gtk_drawing_area_size(
	        GTK_DRAWING_AREA(td->drawing_area),
	        td->cols * td->font_wid,
	        td->rows * td->font_hgt);

	/* Set geometry hints */
	term_data_set_geometry_hints(td);


	/* Install window event handlers */
	gtk_signal_connect(
	        GTK_OBJECT(td->window),
	        "delete_event",
	        GTK_SIGNAL_FUNC(delete_event_handler),
	        NULL);
	gtk_signal_connect(
	        GTK_OBJECT(td->window),
	        "key_press_event",
	        GTK_SIGNAL_FUNC(keypress_event_handler),
	        NULL);

	/* Destroying the Angband window terminates the game */
	if (main_window)
	{
		gtk_signal_connect(
		        GTK_OBJECT(td->window),
		        "destroy_event",
		        GTK_SIGNAL_FUNC(destroy_main_event_handler),
		        NULL);
	}

	/* The other windows are just hidden */
	else
	{
		gtk_signal_connect(
		        GTK_OBJECT(td->window),
		        "destroy_event",
		        GTK_SIGNAL_FUNC(destroy_sub_event_handler),
		        td);
	}


	/* Install drawing area event handlers */
	gtk_signal_connect(
	        GTK_OBJECT(td->drawing_area),
	        "realize",
	        GTK_SIGNAL_FUNC(realize_event_handler),
	        (gpointer)td);
	gtk_signal_connect(
	        GTK_OBJECT(td->drawing_area),
	        "show",
	        GTK_SIGNAL_FUNC(show_event_handler),
	        (gpointer)td);
	gtk_signal_connect(
	        GTK_OBJECT(td->drawing_area),
	        "hide",
	        GTK_SIGNAL_FUNC(hide_event_handler),
	        (gpointer)td);
	gtk_signal_connect(
	        GTK_OBJECT(td->drawing_area),
	        "size_allocate",
	        GTK_SIGNAL_FUNC(size_allocate_event_handler),
	        (gpointer)td);
	gtk_signal_connect(
	        GTK_OBJECT(td->drawing_area),
	        "expose_event",
	        GTK_SIGNAL_FUNC(expose_event_handler),
	        (gpointer)td);


	/* Create menu */
	if (main_window)
	{
		/* Build the main menu bar */
		menu_bar = get_main_menu(td);
		g_assert(menu_bar != NULL);

		/* Since it's tedious to scatter the menu update code around */
		add_menu_update_callbacks();
	}


	/* Pack the menu bar together with the main window */
	/* For vertical placement of the menu bar and the drawing area */
	box = gtk_vbox_new(FALSE, 0);

	/* Let the window widget own it */
	gtk_container_add(GTK_CONTAINER(td->window), box);

	/* The main window has a menu bar */
	if (main_window)
		gtk_box_pack_start(
		        GTK_BOX(box),
		        menu_bar,
		        FALSE,
		        FALSE,
		        NO_PADDING);

	/* And place the drawing area just beneath it */
	gtk_box_pack_start_defaults(GTK_BOX(box), td->drawing_area);


	/* Show the widgets - use of td->shown is a dirty hack XXX XXX */
	if (td->shown) gtk_widget_show_all(td->window);
}


/*
 * To be hooked into quit(). See z-util.c
 */
static void hook_quit(const char *str)
{
	/* Free menu paths dynamically allocated */
	free_menu_paths();


	/* Terminate the program */
	gtk_exit(0);
}


/*
 * Initialization function
 */
int init_gtk2(int argc, char **argv)
{
	int i;


	/* Initialize the environment */
	gtk_init(&argc, &argv);

	/* Activate hooks - Use gtk/glib interface throughout */
	quit_aux = hook_quit;

	/* Parse args */
	for (i = 1; i < argc; i++)
	{
		/* Number of terminals displayed at start up */
		if (prefix(argv[i], "-n"))
		{
			num_term = atoi(&argv[i][2]);
			if (num_term > MAX_TERM_DATA) num_term = MAX_TERM_DATA;
			else if (num_term < 1) num_term = 1;
			continue;
		}

		/* Disable use of pixmaps as backing store */
		if (streq(argv[i], "-b"))
		{
			use_backing_store = FALSE;
			continue;
		}


		/* None of the above */
		fprintf(stderr, "Ignoring option: %s", argv[i]);
	}


	/* Initialise colours */
	gdk_rgb_init();
	gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
	gtk_widget_set_default_visual(gdk_rgb_get_visual());
	init_colours();

	/*
	 * Initialise the windows backwards, so that
	 * the Angband window comes in front
	 */
	for (i = MAX_TERM_DATA - 1; i >= 0; i--)
	{
		term_data *td = &data[i];

		/* Initialize the term_data */
		term_data_init(td, i);

		/* Hack - Set the shown flag, meaning "to be shown" XXX XXX */
		if (i < num_term) td->shown = TRUE;
		else td->shown = FALSE;

		/* Save global entry */
		angband_term[i] = Term;

		/* Init the window */
		init_gtk_window(td, i);
	}

	/* Activate the "Angband" window screen */
	Term_activate(data[0].term_ptr);

	/* Activate more hook */
	plog_aux = hook_plog;

	/* It's too early to set this, but cannot do so elsewhere XXX XXX */
	game_in_progress = TRUE;

	/* Success */
	return (0);
}

/**
 * Main
 */
int main(int argc, char *argv[])
{
	return main_real(
		argc,
		argv,
		"gtk2",
		init_gtk2,
		// Usage:
		"  -- -n#             Number of terms to use\n"
		"  -- -b              Turn off software backing store\n");
}

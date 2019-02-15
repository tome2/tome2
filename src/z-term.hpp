/*
 * Copyright (c) 1997 Ben Harrison
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */

#pragma once

#include "h-basic.hpp"

#include <functional>

typedef struct term_win term_win;

struct term_win; // Opaque

typedef struct term term;

struct term; // Opaque

/*
 * UI hooks
 */

typedef void(init_hook_t)(void *data);
typedef void(nuke_hook_t)(void *data);
typedef void(xtra_hook_t)(void *data, int n, int v);
typedef void(curs_hook_t)(void *data, int x, int y);
typedef void(text_hook_t)(void *data, int x, int y, int n, byte a, const char *s);

typedef struct term_ui_hooks_t term_ui_hooks_t;

struct term_ui_hooks_t {

	init_hook_t *init_hook;

	nuke_hook_t *nuke_hook;

	xtra_hook_t *xtra_hook;

	curs_hook_t *curs_hook;

	text_hook_t *text_hook;

};

/*
 * Resize hook type
 */
typedef void(resize_hook_t)();


/*** Color constants ***/


/*
 * Angband "attributes" (with symbols, and base (R,G,B) codes)
 *
 * The "(R,G,B)" codes are given in "fourths" of the "maximal" value,
 * and should "gamma corrected" on most (non-Macintosh) machines.
 */
#define TERM_DARK                0  /* 'd' */   /* 0,0,0 */
#define TERM_WHITE               1  /* 'w' */   /* 4,4,4 */
#define TERM_SLATE               2  /* 's' */   /* 2,2,2 */
#define TERM_ORANGE              3  /* 'o' */   /* 4,2,0 */
#define TERM_RED                 4  /* 'r' */   /* 3,0,0 */
#define TERM_GREEN               5  /* 'g' */   /* 0,2,1 */
#define TERM_BLUE                6  /* 'b' */   /* 0,0,4 */
#define TERM_UMBER               7  /* 'u' */   /* 2,1,0 */
#define TERM_L_DARK              8  /* 'D' */   /* 1,1,1 */
#define TERM_L_WHITE             9  /* 'W' */   /* 3,3,3 */
#define TERM_VIOLET             10  /* 'v' */   /* 4,0,4 */
#define TERM_YELLOW             11  /* 'y' */   /* 4,4,0 */
#define TERM_L_RED              12  /* 'R' */   /* 4,0,0 */
#define TERM_L_GREEN            13  /* 'G' */   /* 0,4,0 */
#define TERM_L_BLUE             14  /* 'B' */   /* 0,4,4 */
#define TERM_L_UMBER            15  /* 'U' */   /* 3,2,1 */



/**** Available Constants ****/


/*
 * Definitions for the "actions" of "Term_xtra()"
 *
 * These values may be used as the first parameter of "Term_xtra()",
 * with the second parameter depending on the "action" itself.  Many
 * of the actions shown below are optional on at least one platform.
 *
 * The "TERM_XTRA_EVENT" action uses "v" to "wait" for an event
 * The "TERM_XTRA_SHAPE" action uses "v" to "show" the cursor
 * The "TERM_XTRA_ALIVE" action uses "v" to "activate" (or "close")
 * The "TERM_XTRA_LEVEL" action uses "v" to "resume" (or "suspend")
 *
 * The other actions do not need a "v" code, so "zero" is used.
 */
#define TERM_XTRA_EVENT	1	/* Process some pending events */
#define TERM_XTRA_FLUSH 2	/* Flush all pending events */
#define TERM_XTRA_CLEAR 3	/* Clear the entire window */
#define TERM_XTRA_FRESH 6	/* Flush all rows (optional) */
#define TERM_XTRA_NOISE 7	/* Make a noise (optional) */
#define TERM_XTRA_BORED 9	/* Handle stuff when bored (optional) */
#define TERM_XTRA_REACT 10	/* React to global changes (optional) */
#define TERM_XTRA_LEVEL 12	/* Change the "soft" level (optional) */
#define TERM_XTRA_RENAME_MAIN_WIN 16 /* Rename the main game window */


/**** Available Variables ****/

extern term *Term;

/**** Available Functions ****/

void Term_xtra(int n, int v);

void Term_queue_char(int x, int y, byte a, char c);

void Term_fresh();
errr Term_gotoxy(int x, int y);
void Term_draw(int x, int y, byte a, char c);
void Term_addch(byte a, char c);
errr Term_addstr(int n, byte a, const char *s);
void Term_putch(int x, int y, byte a, char c);
void Term_putstr(int x, int y, int n, byte a, const char *s);
void Term_erase(int x, int y, int n);
void Term_clear();
void Term_redraw();
void Term_redraw_section(int x1, int y1, int x2, int y2);
void Term_bell();

void Term_with_saved_cursor_flags(std::function<void ()> callback);
void Term_with_saved_cursor_visbility(std::function<void ()> callback);

void Term_get_size(int *w, int *h);
void Term_locate(int *x, int *y);
void Term_what(int x, int y, byte *a, char *c);

void Term_show_cursor();
void Term_hide_cursor();

void Term_flush();
void Term_keypress(int k);
errr Term_key_push(int k);
errr Term_inkey(char *ch, bool wait, bool take);

void Term_save();
term_win* Term_save_to();
void Term_load();
void Term_load_from(term_win *save);

void Term_resize(int w, int h);

void Term_activate(term *t);

void Term_mapped();
void Term_unmapped();

void term_nuke(term *t);
term *term_init(void *data, int w, int h, int k);
void term_init_icky_corner(term *t);
void term_init_soft_cursor(term *t);
void term_init_ui_hooks(term *t, term_ui_hooks_t hooks);
void term_set_resize_hook(term *t, resize_hook_t *hook);
void term_get_size(term *t, int *w, int *h);

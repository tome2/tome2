/* File: main-win.c */

/*
 * Copyright (c) 1997 Ben Harrison, Skirmantas Kligys, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


/*
 * This file helps Angband work with Windows computers.
 *
 * To use this file, use an appropriate "Makefile" or "Project File",
 * make sure that "WINDOWS" and/or "WIN32" are defined somewhere, and
 * make sure to obtain various extra files as described below.
 *
 * The official compilation uses the CodeWarrior Pro compiler, which
 * includes a special project file and precompilable header file.
 *
 *
 * See also "main-dos.c" and "main-ibm.c".
 *
 *
 * The "lib/user/pref-win.prf" file contains keymaps, macro definitions,
 * and/or color redefinitions.
 *
 * The "lib/user/font-win.prf" contains attr/char mappings for use with the
 * normal "lib/xtra/font/ .fon" font files.
 *
 * The "lib/user/graf-win.prf" contains attr/char mappings for use with the
 * special "lib/xtra/graf/ .bmp" bitmap files, which are activated by a
 * menu item.
 *
 *
 * Compiling this file, and using the resulting executable, requires
 * several extra files not distributed with the standard Angband code.
 * In any case, some "*.fon" files (including "8X13.FON" if nothing
 * else) must be placed into "lib/xtra/font/".
 *
 *
 * The "Term_xtra_win_clear()" function should probably do a low-level
 * clear of the current window, and redraw the borders and other things,
 * if only for efficiency.  XXX XXX XXX
 *
 * A simpler method is needed for selecting the "tile size" for windows.
 * XXX XXX XXX
 *
 * The various "warning" messages assume the existance of the "screen.w"
 * window, I think, and only a few calls actually check for its existance,
 * this may be okay since "NULL" means "on top of all windows". (?)  The
 * user must never be allowed to "hide" the main window, or the "menubar"
 * will disappear.  XXX XXX XXX
 *
 * Special "Windows Help Files" can be placed into "lib/xtra/help/" for
 * use with the "winhelp.exe" program.  These files *may* be available
 * at the ftp site somewhere, but I have not seen them.  XXX XXX XXX
 *
 *
 * Initial framework (and most code) by Ben Harrison (benh@phial.com).
 *
 * Original code by Skirmantas Kligys (kligys@scf.usc.edu).
 *
 * Additional code by Ross E Becker (beckerr@cis.ohio-state.edu),
 * and Chris R. Martin (crm7479@tam2000.tamu.edu).
 */


#include "angband.h"
#include "config.h"
#include "defines.h"
#include "files.h"
#include "h-basic.hpp"
#include "util.h"
#include "variable.h"
#include "z-form.h"
#include "z-term.hpp"
#include "z-util.h"

/*
 * Determine if string "t" is a suffix of string "s"
 */
static bool suffix(const char *s, const char *t)
{
	int tlen = strlen(t);
	int slen = strlen(s);

	/* Check for incompatible lengths */
	if (tlen > slen) return false;

	/* Compare "t" to the end of "s" */
	return !strcmp(s + slen - tlen, t);
}


/*
 * Extract the "WIN32" flag from the compiler
 */
#if defined(__WIN32__) || defined(__WINNT__) || defined(__NT__)
# ifndef WIN32
# define WIN32
# endif 
#endif


/*
 * Hack -- allow use of "screen saver" mode
 */
#define USE_SAVER

/*
 * Menu constants -- see "ANGBAND.RC"
 */

#define IDM_FILE_NEW			100
#define IDM_FILE_OPEN			101
#define IDM_FILE_SAVE			110
#define IDM_FILE_SCORE	120
#define IDM_FILE_EXIT			121

#define IDM_WINDOW_VIS_0		200
#define IDM_WINDOW_VIS_1		201
#define IDM_WINDOW_VIS_2		202
#define IDM_WINDOW_VIS_3		203
#define IDM_WINDOW_VIS_4		204
#define IDM_WINDOW_VIS_5		205
#define IDM_WINDOW_VIS_6		206
#define IDM_WINDOW_VIS_7		207

#define IDM_WINDOW_FONT_0		210
#define IDM_WINDOW_FONT_1		211
#define IDM_WINDOW_FONT_2		212
#define IDM_WINDOW_FONT_3		213
#define IDM_WINDOW_FONT_4		214
#define IDM_WINDOW_FONT_5		215
#define IDM_WINDOW_FONT_6		216
#define IDM_WINDOW_FONT_7		217

#define IDM_WINDOW_BIZ_0		230
#define IDM_WINDOW_BIZ_1		231
#define IDM_WINDOW_BIZ_2		232
#define IDM_WINDOW_BIZ_3		233
#define IDM_WINDOW_BIZ_4		234
#define IDM_WINDOW_BIZ_5		235
#define IDM_WINDOW_BIZ_6		236
#define IDM_WINDOW_BIZ_7		237

#define IDM_WINDOW_I_WID_0		240
#define IDM_WINDOW_I_WID_1		241
#define IDM_WINDOW_I_WID_2		242
#define IDM_WINDOW_I_WID_3		243
#define IDM_WINDOW_I_WID_4		244
#define IDM_WINDOW_I_WID_5		245
#define IDM_WINDOW_I_WID_6		246
#define IDM_WINDOW_I_WID_7		247

#define IDM_WINDOW_D_WID_0		250
#define IDM_WINDOW_D_WID_1		251
#define IDM_WINDOW_D_WID_2		252
#define IDM_WINDOW_D_WID_3		253
#define IDM_WINDOW_D_WID_4		254
#define IDM_WINDOW_D_WID_5		255
#define IDM_WINDOW_D_WID_6		256
#define IDM_WINDOW_D_WID_7		257

#define IDM_WINDOW_I_HGT_0		260
#define IDM_WINDOW_I_HGT_1		261
#define IDM_WINDOW_I_HGT_2		262
#define IDM_WINDOW_I_HGT_3		263
#define IDM_WINDOW_I_HGT_4		264
#define IDM_WINDOW_I_HGT_5		265
#define IDM_WINDOW_I_HGT_6		266
#define IDM_WINDOW_I_HGT_7		267

#define IDM_WINDOW_D_HGT_0		270
#define IDM_WINDOW_D_HGT_1		271
#define IDM_WINDOW_D_HGT_2		272
#define IDM_WINDOW_D_HGT_3		273
#define IDM_WINDOW_D_HGT_4		274
#define IDM_WINDOW_D_HGT_5		275
#define IDM_WINDOW_D_HGT_6		276
#define IDM_WINDOW_D_HGT_7		277

#define IDM_OPTIONS_UNUSED		410
#define IDM_OPTIONS_SAVER		411


/*
 * This may need to be removed for some compilers XXX XXX XXX
 */
#define STRICT

/*
 * Exclude parts of WINDOWS.H that are not needed
 */
#define NOCOMM            /* Comm driver APIs and definitions */
#define NOLOGERROR        /* LogError() and related definitions */
#define NOPROFILER        /* Profiler APIs */
#define NOLFILEIO         /* _l* file I/O routines */
#define NOOPENFILE        /* OpenFile and related definitions */
#define NORESOURCE        /* Resource management */
#define NOATOM            /* Atom management */
#define NOLANGUAGE        /* Character test routines */
#define NOLSTRING         /* lstr* string management routines */
#define NODBCS            /* Double-byte character set routines */
#define NOKEYBOARDINFO    /* Keyboard driver routines */
#define NOCOLOR           /* COLOR_* color values */
#define NODRAWTEXT        /* DrawText() and related definitions */
#define NOSCALABLEFONT    /* Truetype scalable font support */
#define NOMETAFILE        /* Metafile support */
#define NOSYSTEMPARAMSINFO /* SystemParametersInfo() and SPI_* definitions */
#define NODEFERWINDOWPOS  /* DeferWindowPos and related definitions */
#define NOKEYSTATES       /* MK_* message key state flags */
#define NOWH              /* SetWindowsHook and related WH_* definitions */
#define NOCLIPBOARD       /* Clipboard APIs and definitions */
#define NOICONS           /* IDI_* icon IDs */
#define NOMDI             /* MDI support */
#define NOHELP            /* Help support */

/* Not defined since it breaks Borland C++ 5.5 */
/* #define NOCTLMGR */    /* Control management and controls */

/*
 * Exclude parts of WINDOWS.H that are not needed (Win32)
 */
#define WIN32_LEAN_AND_MEAN
#define NONLS             /* All NLS defines and routines */
#define NOSERVICE         /* All Service Controller routines, SERVICE_ equates, etc. */
#define NOKANJI           /* Kanji support stuff. */
#define NOMCX             /* Modem Configuration Extensions */

/*
 * Include the "windows" support file
 */
#include <windows.h>


/*
 * Exclude parts of MMSYSTEM.H that are not needed
 */
#define MMNODRV          /* Installable driver support */
#define MMNOWAVE         /* Waveform support */
#define MMNOMIDI         /* MIDI support */
#define MMNOAUX          /* Auxiliary audio support */
#define MMNOJOY          /* Joystick support */
#define MMNOMCI          /* MCI support */
#define MMNOMMIO         /* Multimedia file I/O support */
#define MMNOMMSYSTEM     /* General MMSYSTEM functions */

/*
 * Include some more files
 */
#include <mmsystem.h>
#include <commdlg.h>

/*
 * Include the support for loading bitmaps
 */

/*
 * Hack -- Fake declarations from "dos.h" XXX XXX XXX
 */
#ifdef WIN32
#define INVALID_FILE_NAME (DWORD)0xFFFFFFFF
#else /* WIN32 */
#define FA_LABEL    0x08        /* Volume label */
#define FA_DIREC    0x10        /* Directory */
unsigned _cdecl _dos_getfileattr(const char *, unsigned *);
#endif /* WIN32 */

/*
 * Silliness in WIN32 drawing routine
 */
#ifdef WIN32
# define MoveTo(H, X, Y) MoveToEx(H, X, Y, NULL)
#endif /* WIN32 */

/*
 * Silliness for Windows 95
 */
#ifndef WS_EX_TOOLWINDOW
# define WS_EX_TOOLWINDOW 0
#endif

/*
 * Foreground color bits (hard-coded by DOS)
 */
#define VID_BLACK	0x00
#define VID_BLUE	0x01
#define VID_GREEN	0x02
#define VID_CYAN	0x03
#define VID_RED		0x04
#define VID_MAGENTA	0x05
#define VID_YELLOW	0x06
#define VID_WHITE	0x07

/*
 * Bright text (hard-coded by DOS)
 */
#define VID_BRIGHT	0x08

/*
 * Background color bits (hard-coded by DOS)
 */
#define VUD_BLACK	0x00
#define VUD_BLUE	0x10
#define VUD_GREEN	0x20
#define VUD_CYAN	0x30
#define VUD_RED		0x40
#define VUD_MAGENTA	0x50
#define VUD_YELLOW	0x60
#define VUD_WHITE	0x70

/*
 * Blinking text (hard-coded by DOS)
 */
#define VUD_BRIGHT	0x80



/*
 * Forward declare
 */
typedef struct _term_data term_data;

/*
 * Extra "term" data
 *
 * Note the use of "font_want" for the names of the font file requested by
 * the user, and the use of "font_file" for the currently active font file.
 *
 * The "font_file" is uppercased, and takes the form "8X13.FON", while
 * "font_want" can be in almost any form as long as it could be construed
 * as attempting to represent the name of a font.
 */
struct _term_data
{
	term *term_ptr;

	const char *s;

	HWND w;

	DWORD dwStyle;
	DWORD dwExStyle;

	unsigned int keys;

	unsigned int rows;
	unsigned int cols;

	unsigned int pos_x;
	unsigned int pos_y;
	unsigned int size_wid;
	unsigned int size_hgt;
	unsigned int size_ow1;
	unsigned int size_oh1;
	unsigned int size_ow2;
	unsigned int size_oh2;

	bool size_hack;

	bool xtra_hack;

	bool visible;

	bool bizarre;

	const char *font_want;

	const char *font_file;

	HFONT font_id;

	unsigned int font_wid;
	unsigned int font_hgt;

	unsigned int tile_wid;
	unsigned int tile_hgt;
};


/*
 * Maximum number of windows XXX XXX XXX
 */
#define MAX_TERM_DATA 8

/*
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];

/*
 * Hack -- global "window creation" pointer
 */
static term_data *my_td;

/*
 * game in progress
 */
bool game_in_progress = false;

/*
 * note when "open"/"new" become valid
 */
bool initialized = false;

/*
 * screen paletted, i.e. 256 colors
 */
bool paletted = false;

/*
 * 16 colors screen, don't use RGB()
 */
bool colors16 = false;

/*
 * Saved instance handle
 */
static HINSTANCE hInstance;

/*
 * Yellow brush for the cursor
 */
static HBRUSH hbrYellow;

/*
 * An icon
 */
static HICON hIcon;

/*
 * A palette
 */
static HPALETTE hPal;


#ifdef USE_SAVER

/*
 * The screen saver window
 */
static HWND hwndSaver;

#endif /* USE_SAVER */






/*
 * Full path to ANGBAND.INI
 */
static const char *ini_file = NULL;

/*
 * Name of application
 */
static const char *AppName = "ANGBAND";

/*
 * Name of sub-window type
 */
static const char *AngList = "AngList";

/*
 * Directory names
 */
static const char *ANGBAND_DIR_XTRA_FONT;
static const char *ANGBAND_DIR_XTRA_GRAF;
static const char *ANGBAND_DIR_XTRA_HELP;


/*
 * The "complex" color values
 */
static COLORREF win_clr[256];


/*
 * The "simple" color values
 *
 * See "main-ibm.c" for original table information
 *
 * The entries below are taken from the "color bits" defined above.
 *
 * Note that many of the choices below suck, but so do crappy monitors.
 */
static BYTE win_pal[256] =
{
	VID_BLACK, 					/* Dark */
	VID_WHITE, 					/* White */
	VID_CYAN, 					/* Slate XXX */
	VID_RED | VID_BRIGHT, 		/* Orange XXX */
	VID_RED, 					/* Red */
	VID_GREEN, 					/* Green */
	VID_BLUE, 					/* Blue */
	VID_YELLOW, 					/* Umber XXX */
	VID_BLACK | VID_BRIGHT, 		/* Light Dark */
	VID_CYAN | VID_BRIGHT, 		/* Light Slate XXX */
	VID_MAGENTA, 				/* Violet XXX */
	VID_YELLOW | VID_BRIGHT, 	/* Yellow */
	VID_MAGENTA | VID_BRIGHT, 	/* Light Red XXX */
	VID_GREEN | VID_BRIGHT, 		/* Light Green */
	VID_BLUE | VID_BRIGHT, 		/* Light Blue */
	VID_YELLOW					/* Light Umber XXX */
};


/*
 * Hack -- define which keys are "special"
 */
static bool special_key[256];
static bool ignore_key[256];

/*
 * Hack -- initialization list for "special_key"
 */
static byte special_key_list[] = {
	VK_CLEAR, VK_PAUSE, VK_CAPITAL, VK_KANA,
#ifdef JP
	VK_JUNJA, VK_FINAL, VK_KANJI, VK_CONVERT, VK_NONCONVERT, VK_ACCEPT, VK_MODECHANGE,
#endif
	VK_PRIOR, VK_NEXT, VK_END, VK_HOME, VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN,
	VK_SELECT, VK_PRINT, VK_EXECUTE, VK_SNAPSHOT, VK_INSERT, VK_DELETE,
	VK_HELP, VK_APPS,
	VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10,
	VK_F11, VK_F12, VK_F13, VK_F14, VK_F15, VK_F16, VK_F17, VK_F18, VK_F19, VK_F20,
	VK_F21, VK_F22, VK_F23, VK_F24, VK_NUMLOCK, VK_SCROLL,
	VK_ATTN, VK_CRSEL, VK_EXSEL, VK_EREOF, VK_PLAY, VK_ZOOM, VK_NONAME,
	VK_PA1, 0
};

static byte ignore_key_list[] = {
	VK_ESCAPE, VK_TAB, VK_SPACE,
	'F', 'W', 'O', 'H',  /* these are menu characters.*/
	VK_SHIFT, VK_CONTROL, VK_MENU, VK_LWIN, VK_RWIN,
	VK_LSHIFT, VK_RSHIFT, VK_LCONTROL, VK_RCONTROL, VK_LMENU, VK_RMENU, 0
};


/*
 * Hack -- given a pathname, point at the filename
 */
static const char *extract_file_name(const char *s)
{
	const char *p;

	/* Start at the end */
	p = s + strlen(s) - 1;

	/* Back up to divider */
	while ((p >= s) && (*p != ':') && (*p != '\\')) p--;

	/* Return file name */
	return (p + 1);
}


/*
 * Hack -- given a simple filename, extract the "font size" info
 *
 * Return a pointer to a static buffer holding the capitalized base name.
 */
static char *analyze_font(char *path, int *wp, int *hp)
{
	int wid, hgt;

	char *s, *p;

	/* Start at the end */
	p = path + strlen(path) - 1;

	/* Back up to divider */
	while ((p >= path) && (*p != ':') && (*p != '\\')) --p;

	/* Advance to file name */
	++p;

	/* Capitalize */
	for (s = p; *s; ++s)
	{
		/* Capitalize (be paranoid) */
		if (islower(*s)) *s = toupper(*s);
	}

	/* Find first 'X' */
	s = strchr(p, 'X');

	/* Extract font width */
	wid = atoi(p);

	/* Extract height */
	hgt = s ? atoi(s + 1) : 0;

	/* Save results */
	(*wp) = wid;
	(*hp) = hgt;

	/* Result */
	return (p);
}


/*
 * Check for existance of a file
 */
static bool check_file(const char *s)
{
	char path[1024];

#ifdef WIN32

	DWORD attrib;

#else /* WIN32 */

	unsigned int attrib;

#endif /* WIN32 */

	/* Copy it */
	strcpy(path, s);

#ifdef WIN32

	/* Examine */
	attrib = GetFileAttributes(path);

	/* Require valid filename */
	if (attrib == INVALID_FILE_NAME) return false;

	/* Prohibit directory */
	if (attrib & FILE_ATTRIBUTE_DIRECTORY) return false;

#else /* WIN32 */

	/* Examine and verify */
	if (_dos_getfileattr(path, &attrib)) return false;

	/* Prohibit something */
	if (attrib & FA_LABEL) return false;

	/* Prohibit directory */
	if (attrib & FA_DIREC) return false;

#endif /* WIN32 */

	/* Success */
	return true;
}


/*
 * Check for existance of a directory
 */
static bool check_dir(const char *s)
{
	int i;

	char path[1024];

#ifdef WIN32

	DWORD attrib;

#else /* WIN32 */

	unsigned int attrib;

#endif /* WIN32 */

	/* Copy it */
	strcpy(path, s);

	/* Check length */
	i = strlen(path);

	/* Remove trailing backslash */
	if (i && (path[i - 1] == '\\')) path[--i] = '\0';

#ifdef WIN32

	/* Examine */
	attrib = GetFileAttributes(path);

	/* Require valid filename */
	if (attrib == INVALID_FILE_NAME) return false;

	/* Require directory */
	if (!(attrib & FILE_ATTRIBUTE_DIRECTORY)) return false;

#else /* WIN32 */

	/* Examine and verify */
	if (_dos_getfileattr(path, &attrib)) return false;

	/* Prohibit something */
	if (attrib & FA_LABEL) return false;

	/* Require directory */
	if (!(attrib & FA_DIREC)) return false;

#endif /* WIN32 */

	/* Success */
	return true;
}


/*
 * Validate a file
 */
static void validate_file(const char *s)
{
	/* Verify or fail */
	if (!check_file(s))
	{
		quit_fmt("Cannot find required file:\n%s", s);
	}
}


/*
 * Validate a directory
 */
static void validate_dir(const char *s)
{
	/* Verify or fail */
	if (!check_dir(s))
	{
		quit_fmt("Cannot find required directory:\n%s", s);
	}
}


/*
 * Get the "size" for a window
 */
static void term_getsize(term_data *td)
{
	RECT rc;

	int wid, hgt;

	/*
	 * The Angband window is always at least 80x24. I'm not sure if
	 * small windows work... -- pelpel
	 */
	if (td == &data[0])
	{
		if (td->cols < 80) td->cols = 80;
		if (td->rows < 24) td->rows = 24;
	}

	/* The other windows can be smaller */
	else
	{
		/* Paranoia */
		if (td->cols < 1) td->cols = 1;
		if (td->rows < 1) td->rows = 1;
	}

	/* Paranoia */
	if (td->cols > 255) td->cols = 255;
	if (td->rows > 255) td->rows = 255;

	/* Window sizes */
	wid = td->cols * td->tile_wid + td->size_ow1 + td->size_ow2;
	hgt = td->rows * td->tile_hgt + td->size_oh1 + td->size_oh2;

	/* Fake window size */
	rc.left = 0;
	rc.right = rc.left + wid;
	rc.top = 0;
	rc.bottom = rc.top + hgt;

	/* XXX XXX XXX */
	/* rc.right += 1; */
	/* rc.bottom += 1; */

	/* Adjust */
	AdjustWindowRectEx(&rc, td->dwStyle, true, td->dwExStyle);

	/* Total size */
	td->size_wid = rc.right - rc.left;
	td->size_hgt = rc.bottom - rc.top;

	/* See CreateWindowEx */
	if (!td->w) return;

	/* Extract actual location */
	GetWindowRect(td->w, &rc);

	/* Save the location */
	td->pos_x = rc.left;
	td->pos_y = rc.top;
}


/*
 * Write the "prefs" for a single term
 */
static void save_prefs_aux(term_data *td, const char *sec_name)
{
	char buf[1024];

	RECT rc;

	/* Paranoia */
	if (!td->w) return;

	/* Visible */
	strcpy(buf, td->visible ? "1" : "0");
	WritePrivateProfileString(sec_name, "Visible", buf, ini_file);

	/* Font */
	strcpy(buf, td->font_file ? td->font_file : "8X13.FON");
	WritePrivateProfileString(sec_name, "Font", buf, ini_file);

	/* Bizarre */
	strcpy(buf, td->bizarre ? "1" : "0");
	WritePrivateProfileString(sec_name, "Bizarre", buf, ini_file);

	/* Tile size (x) */
	wsprintf(buf, "%d", td->tile_wid);
	WritePrivateProfileString(sec_name, "TileWid", buf, ini_file);

	/* Tile size (y) */
	wsprintf(buf, "%d", td->tile_hgt);
	WritePrivateProfileString(sec_name, "TileHgt", buf, ini_file);

	/* Window size (x) */
	wsprintf(buf, "%d", td->cols);
	WritePrivateProfileString(sec_name, "NumCols", buf, ini_file);

	/* Window size (y) */
	wsprintf(buf, "%d", td->rows);
	WritePrivateProfileString(sec_name, "NumRows", buf, ini_file);

	/* Acquire position */
	GetWindowRect(td->w, &rc);

	/* Window position (x) */
	wsprintf(buf, "%d", rc.left);
	WritePrivateProfileString(sec_name, "PositionX", buf, ini_file);

	/* Window position (y) */
	wsprintf(buf, "%d", rc.top);
	WritePrivateProfileString(sec_name, "PositionY", buf, ini_file);
}


/*
 * Write the "prefs"
 *
 * We assume that the windows have all been initialized
 */
static void save_prefs(void)
{
	int i;

	char buf[128];

	/* Save window prefs */
	for (i = 0; i < MAX_TERM_DATA; ++i)
	{
		term_data *td = &data[i];

		sprintf(buf, "Term-%d", i);

		save_prefs_aux(td, buf);
	}
}


/*
 * Load the "prefs" for a single term
 */
static void load_prefs_aux(term_data *td, const char *sec_name)
{
	char tmp[1024];

	int wid, hgt;

	/* Visible */
	td->visible = (GetPrivateProfileInt(sec_name, "Visible", td->visible, ini_file) != 0);

	/* Desired font, with default */
	GetPrivateProfileString(sec_name, "Font", "8X13.FON", tmp, 127, ini_file);

	/* Bizarre */
	td->bizarre = (GetPrivateProfileInt(sec_name, "Bizarre", td->bizarre, ini_file) != 0);

	/* Analyze font, save desired font name */
	td->font_want = strdup(analyze_font(tmp, &wid, &hgt));

	/* Tile size */
	td->tile_wid = GetPrivateProfileInt(sec_name, "TileWid", wid, ini_file);
	td->tile_hgt = GetPrivateProfileInt(sec_name, "TileHgt", hgt, ini_file);

	/* Window size */
	td->cols = GetPrivateProfileInt(sec_name, "NumCols", td->cols, ini_file);
	td->rows = GetPrivateProfileInt(sec_name, "NumRows", td->rows, ini_file);

	/* Window position */
	td->pos_x = GetPrivateProfileInt(sec_name, "PositionX", td->pos_x, ini_file);
	td->pos_y = GetPrivateProfileInt(sec_name, "PositionY", td->pos_y, ini_file);
}


/*
 * Load the "prefs"
 */
static void load_prefs(void)
{
	int i;

	char buf[1024];

	/* Load window prefs */
	for (i = 0; i < MAX_TERM_DATA; ++i)
	{
		term_data *td = &data[i];

		sprintf(buf, "Term-%d", i);

		load_prefs_aux(td, buf);
	}
}


/*
 * Create the new global palette based on the bitmap palette
 * (if any), and the standard 16 entry palette derived from
 * "win_clr[]" which is used for the basic 16 Angband colors.
 *
 * This function is never called before all windows are ready.
 *
 * This function returns false if the new palette could not be
 * prepared, which should normally be a fatal error.  XXX XXX
 *
 * Note that only some machines actually use a "palette".
 */
static int new_palette(void)
{
	HPALETTE hBmPal;
	HPALETTE hNewPal;
	HDC hdc;
	int i, nEntries;
	int pLogPalSize;
	int lppeSize;
	LPLOGPALETTE pLogPal;
	LPPALETTEENTRY lppe;

	term_data *td;


	/* This makes no sense */
	if (!paletted) return true;


	/* No palette */
	hBmPal = NULL;

	/* No bitmap */
	lppeSize = 0;
	lppe = NULL;
	nEntries = 0;


	/* Size of palette */
	pLogPalSize = sizeof(LOGPALETTE) + (nEntries + 16) * sizeof(PALETTEENTRY);

	/* Allocate palette */
	pLogPal = (LPLOGPALETTE) calloc(1, pLogPalSize);
	if (pLogPal == NULL)
	{
		abort();
	}

	/* Version */
	pLogPal->palVersion = 0x300;

	/* Make room for bitmap and normal data */
	pLogPal->palNumEntries = nEntries + 16;

	/* Save the bitmap data */
	for (i = 0; i < nEntries; i++)
	{
		pLogPal->palPalEntry[i] = lppe[i];
	}

	/* Save the normal data */
	for (i = 0; i < 16; i++)
	{
		LPPALETTEENTRY p;

		/* Access the entry */
		p = &(pLogPal->palPalEntry[i + nEntries]);

		/* Save the colors */
		p->peRed = GetRValue(win_clr[i]);
		p->peGreen = GetGValue(win_clr[i]);
		p->peBlue = GetBValue(win_clr[i]);

		/* Save the flags */
		p->peFlags = PC_NOCOLLAPSE;
	}

	/* Free something */
	if (lppe)
	{
		free(lppe);
		lppe = NULL;
	}

	/* Create a new palette, or fail */
	hNewPal = CreatePalette(pLogPal);
	if (!hNewPal) quit("Cannot create palette!");

	/* Free the palette */
	free(pLogPal);
	pLogPal = NULL;

	/* Main window */
	td = &data[0];

	/* Realize the palette */
	hdc = GetDC(td->w);
	SelectPalette(hdc, hNewPal, 0);
	i = RealizePalette(hdc);
	ReleaseDC(td->w, hdc);
	if (i == 0) quit("Cannot realize palette!");

	/* Sub-windows */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		td = &data[i];

		hdc = GetDC(td->w);
		SelectPalette(hdc, hNewPal, 0);
		ReleaseDC(td->w, hdc);
	}

	/* Delete old palette */
	if (hPal) DeleteObject(hPal);

	/* Save new palette */
	hPal = hNewPal;

	/* Success */
	return true;
}



/*
 * Resize a window
 */
static void term_window_resize(term_data *td)
{
	/* Require window */
	if (!td->w) return;

	/* Resize the window */
	SetWindowPos(td->w, 0, 0, 0,
	             td->size_wid, td->size_hgt,
	             SWP_NOMOVE | SWP_NOZORDER);

	/* Redraw later */
	InvalidateRect(td->w, NULL, true);
}



/*
 * Force the use of a new "font file" for a term_data
 *
 * This function may be called before the "window" is ready
 *
 * This function returns zero only if everything succeeds.
 *
 * Note that the "font name" must be capitalized!!!
 */
static errr term_force_font(term_data *td, const char *path)
{
	int i;

	int wid, hgt;

	char *base;

	char buf[1024];


	/* Forget the old font (if needed) */
	if (td->font_id) DeleteObject(td->font_id);

	/* Forget old font */
	if (td->font_file)
	{
		bool used = false;

		/* Scan windows */
		for (i = 0; i < MAX_TERM_DATA; i++)
		{
			/* Check "screen" */
			if ((td != &data[i]) &&
			                (data[i].font_file) &&
					(strcmp(data[i].font_file, td->font_file) == 0))
			{
				used = true;
			}
		}

		/* Remove unused font resources */
		if (!used) RemoveFontResource(td->font_file);

		/* Free the old name */
		free(td->font_file);
		td->font_file = NULL;
	}


	/* No path given */
	if (!path) return (1);


	/* Local copy */
	strcpy(buf, path);

	/* Analyze font path */
	base = analyze_font(buf, &wid, &hgt);

	/* Verify suffix */
	if (!suffix(base, ".FON")) return (1);

	/* Verify file */
	if (!check_file(buf)) return (1);

	/* Load the new font */
	if (!AddFontResource(buf)) return (1);

	/* Save new font name */
	td->font_file = strdup(base);

	/* Remove the "suffix" */
	base[strlen(base) - 4] = '\0';

	/* Create the font (using the 'base' of the font file name!) */
	td->font_id = CreateFont(hgt, wid, 0, 0, FW_DONTCARE, 0, 0, 0,
	                         ANSI_CHARSET, OUT_DEFAULT_PRECIS,
	                         CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
	                         FIXED_PITCH | FF_DONTCARE, base);

	/* Hack -- Unknown size */
	if (!wid || !hgt)
	{
		HDC hdcDesktop;
		HFONT hfOld;
		TEXTMETRIC tm;

		/* all this trouble to get the cell size */
		hdcDesktop = GetDC(HWND_DESKTOP);
		hfOld = SelectObject(hdcDesktop, td->font_id);
		GetTextMetrics(hdcDesktop, &tm);
		SelectObject(hdcDesktop, hfOld);
		ReleaseDC(HWND_DESKTOP, hdcDesktop);

		/* Font size info */
		wid = tm.tmAveCharWidth;
		hgt = tm.tmHeight;
	}

	/* Save the size info */
	td->font_wid = wid;
	td->font_hgt = hgt;

	/* Success */
	return (0);
}



/*
 * Allow the user to change the font for this window.
 */
static void term_change_font(term_data *td)
{
	OPENFILENAME ofn;

	char tmp[1024] = "";

	/* Extract a default if possible */
	if (td->font_file) strcpy(tmp, td->font_file);

	/* Ask for a choice */
	memset(&ofn, 0, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = data[0].w;
	ofn.lpstrFilter = "Angband Font Files (*.fon)\0*.fon\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFile = tmp;
	ofn.nMaxFile = 128;
	ofn.lpstrInitialDir = ANGBAND_DIR_XTRA_FONT;
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	ofn.lpstrDefExt = "fon";

	/* Force choice if legal */
	if (GetOpenFileName(&ofn))
	{
		/* Force the font */
		if (term_force_font(td, tmp))
		{
			/* Access the standard font file */
			path_build(tmp, 1024, ANGBAND_DIR_XTRA_FONT, "8X13.FON");

			/* Force the use of that font */
			(void)term_force_font(td, tmp);
		}

		/* Assume not bizarre */
		td->bizarre = false;

		/* Reset the tile info */
		td->tile_wid = td->font_wid;
		td->tile_hgt = td->font_hgt;

		/* Analyze the font */
		term_getsize(td);

		/* Resize the window */
		term_window_resize(td);
	}
}



/*
 * Hack -- redraw a term_data
 */
static void term_data_redraw(term_data *td)
{
	/* Activate the term */
	Term_activate(td->term_ptr);

	/* Redraw the contents */
	Term_redraw();

	/* Restore the term */
	Term_activate(angband_term[0]);
}





/*** Function hooks needed by "Term" ***/


/*
 * React to global changes
 */
static void Term_xtra_win_react(void)
{
	int i;


	/* Simple color */
	if (colors16)
	{
		/* Save the default colors */
		for (i = 0; i < 256; i++)
		{
			/* Simply accept the desired colors */
			win_pal[i] = angband_color_table[i][0];
		}
	}

	/* Complex color */
	else
	{
		COLORREF code;

		byte rv, gv, bv;

		bool change = false;

		/* Save the default colors */
		for (i = 0; i < 256; i++)
		{
			/* Extract desired values */
			rv = angband_color_table[i][1];
			gv = angband_color_table[i][2];
			bv = angband_color_table[i][3];

			/* Extract a full color code */
			code = PALETTERGB(rv, gv, bv);

			/* Activate changes */
			if (win_clr[i] != code)
			{
				/* Note the change */
				change = true;

				/* Apply the desired color */
				win_clr[i] = code;
			}
		}

		/* Activate the palette if needed */
		if (change) (void)new_palette();
	}






	/* Clean up windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		term *old = Term;

		term_data *td = &data[i];

		int wid;
		int hgt;
		term_get_size(td->term_ptr, &wid, &hgt);

		/* Update resized windows */
		if ((td->cols != wid) || (td->rows != hgt))
		{
			/* Activate */
			Term_activate(&td->term_ptr);

			/* Hack -- Resize the term */
			Term_resize(td->cols, td->rows);

			/* Redraw the contents */
			Term_redraw();

			/* Restore */
			Term_activate(old);
		}
	}
}


/*
 * Process at least one event
 */
static void Term_xtra_win_event(int v)
{
	MSG msg;

	/* Wait for an event */
	if (v)
	{
		/* Block */
		if (GetMessage(&msg, NULL, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	/* Check for an event */
	else
	{
		/* Check */
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
}


/*
 * Process all pending events
 */
static void Term_xtra_win_flush(void)
{
	MSG msg;

	/* Process all pending events */
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}


/*
 * Hack -- clear the screen
 *
 * Make this more efficient XXX XXX XXX
 */
static void Term_xtra_win_clear(term_data *td, void)
{
	HDC hdc;
	RECT rc;

	/* Rectangle to erase */
	rc.left = td->size_ow1;
	rc.right = rc.left + td->cols * td->tile_wid;
	rc.top = td->size_oh1;
	rc.bottom = rc.top + td->rows * td->tile_hgt;

	/* Erase it */
	hdc = GetDC(td->w);
	SetBkColor(hdc, RGB(0, 0, 0));
	SelectObject(hdc, td->font_id);
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);
	ReleaseDC(td->w, hdc);
}


/*
 * Hack -- make a noise
 */
static void Term_xtra_win_noise(void)
{
	MessageBeep(MB_ICONASTERISK);
}


/*
 * Delay for "x" milliseconds
 */
static void Term_xtra_win_delay(int v)
{

#ifdef WIN32

	/* Sleep */
	Sleep(v);

#else /* WIN32 */

	DWORD t;
	MSG msg;

	/* Final count */
	t = GetTickCount() + v;

	/* Wait for it */
	while (GetTickCount() < t)
	{
		/* Handle messages */
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

#endif /* WIN32 */
}

/*
 * Return the HWND of the main window
 */
HWND get_main_hwnd()
{
	return data[0].w;
}

/*
 * Do a "special thing"
 */
static void Term_xtra_win(void *data, int n, int v)
{
	term_data *td = (term_data*) data;

	/* Handle a subset of the legal requests */
	switch (n)
	{
		/* Make a bell sound */
	case TERM_XTRA_NOISE:
		{
			Term_xtra_win_noise();
			return;
		}

		/* Process random events */
	case TERM_XTRA_BORED:
		{
			Term_xtra_win_event(0);
			return;
		}

		/* Process an event */
	case TERM_XTRA_EVENT:
		{
			Term_xtra_win_event(v);
			return;
		}

		/* Flush all events */
	case TERM_XTRA_FLUSH:
		{
			Term_xtra_win_flush();
			return;
		}

		/* Clear the screen */
	case TERM_XTRA_CLEAR:
		{
			Term_xtra_win_clear(td);
			return;
		}

		/* React to global changes */
	case TERM_XTRA_REACT:
		{
			Term_xtra_win_react();
			return;
		}

		/* Delay for some milliseconds */
	case TERM_XTRA_DELAY:
		{
			Term_xtra_win_delay(v);
			return;
		}

		/* Rename main window */
	case TERM_XTRA_RENAME_MAIN_WIN:
		{
			SetWindowText(get_main_hwnd(), angband_term_name[0]);
			return;
		}
	}
}



/*
 * Low level graphics (Assumes valid input).
 *
 * Draw a "cursor" at (x,y), using a "yellow box".
 */
static void Term_curs_win(void *data, int x, int y)
{
	term_data *td = (term_data*) data;

	RECT rc;
	HDC hdc;

	/* Frame the grid */
	rc.left = x * td->tile_wid + td->size_ow1;
	rc.right = rc.left + td->tile_wid;
	rc.top = y * td->tile_hgt + td->size_oh1;
	rc.bottom = rc.top + td->tile_hgt;

	/* Cursor is done as a yellow "box" */
	hdc = GetDC(data[0].w);
	FrameRect(hdc, &rc, hbrYellow);
	ReleaseDC(data[0].w, hdc);
}


/*
 * Low level graphics.  Assumes valid input.
 *
 * Draw several ("n") chars, with an attr, at a given location.
 *
 * All "graphic" data is handled by "Term_pict_win()", below.
 *
 * One would think there is a more efficient method for telling a window
 * what color it should be using to draw with, but perhaps simply changing
 * it every time is not too inefficient.  XXX XXX XXX
 */
static void Term_text_win(void *data, int x, int y, int n, byte a, const char *s)
{
	term_data *td = (term_data*) data;
	RECT rc;
	HDC hdc;


	/* Total rectangle */
	rc.left = x * td->tile_wid + td->size_ow1;
	rc.right = rc.left + n * td->tile_wid;
	rc.top = y * td->tile_hgt + td->size_oh1;
	rc.bottom = rc.top + td->tile_hgt;

	/* Acquire DC */
	hdc = GetDC(td->w);

	/* Background color */
	SetBkColor(hdc, RGB(0, 0, 0));

	/* Foreground color */
	if (colors16)
	{
		SetTextColor(hdc, PALETTEINDEX(win_pal[a]));
	}
	else if (paletted)
	{
		SetTextColor(hdc, win_clr[a&0x0F]);
	}
	else
	{
		SetTextColor(hdc, win_clr[a]);
	}

	/* Use the font */
	SelectObject(hdc, td->font_id);

	/* Bizarre size */
	if (td->bizarre ||
	                (td->tile_hgt != td->font_hgt) ||
	                (td->tile_wid != td->font_wid))
	{
		int i;

		/* Erase complete rectangle */
		ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &rc, NULL, 0, NULL);

		/* New rectangle */
		rc.left += ((td->tile_wid - td->font_wid) / 2);
		rc.right = rc.left + td->font_wid;
		rc.top += ((td->tile_hgt - td->font_hgt) / 2);
		rc.bottom = rc.top + td->font_hgt;

		/* Dump each character */
		for (i = 0; i < n; i++)
		{
			/* Dump the text */
			ExtTextOut(hdc, rc.left, rc.top, 0, &rc,
			           s + i, 1, NULL);

			/* Advance */
			rc.left += td->tile_wid;
			rc.right += td->tile_wid;
		}
	}

	/* Normal size */
	else
	{
		/* Dump the text */
		ExtTextOut(hdc, rc.left, rc.top, ETO_OPAQUE | ETO_CLIPPED, &rc,
		           s, n, NULL);
	}

	/* Release DC */
	ReleaseDC(td->w, hdc);
}



/*** Other routines ***/


/*
 * Create and initialize a "term_data" given a title
 */
static void term_data_link(term_data *td)
{
	/* Hooks */
	struct term_ui_hooks_t ui_hooks = {
		NULL /* init */,
		NULL /* nuke */,
		Term_xtra_win,
		Term_curs_win,
		Term_text_win,
	};

	/* Initialize the term */
	td->term_ptr = term_init(td, td->cols, td->rows, td->keys);
	term_init_soft_cursor(td->term_ptr);
	term_init_ui_hooks(td->term_ptr, ui_hooks);
}


/*
 * Create the windows
 *
 * First, instantiate the "default" values, then read the "ini_file"
 * to over-ride selected values, then create the windows, and fonts.
 *
 * Must use SW_SHOW not SW_SHOWNA, since on 256 color display
 * must make active to realize the palette.  XXX XXX XXX
 */
static void init_windows(void)
{
	int i;

	term_data *td;

	char buf[1024];


	/* Main window */
	td = &data[0];
	memset(td, 0, sizeof(term_data));
	td->s = angband_term_name[0];
	td->keys = 1024;
	td->rows = 24;
	td->cols = 80;
	td->visible = true;
	td->size_ow1 = 2;
	td->size_ow2 = 2;
	td->size_oh1 = 2;
	td->size_oh2 = 2;
	td->pos_x = 7 * 30;
	td->pos_y = 7 * 20;

	/* Sub windows */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		td = &data[i];
		memset(td, 0, sizeof(term_data));
		td->s = angband_term_name[i];
		td->keys = 16;
		td->rows = 24;
		td->cols = 80;
		td->visible = false;
		td->size_ow1 = 1;
		td->size_ow2 = 1;
		td->size_oh1 = 1;
		td->size_oh2 = 1;
		td->pos_x = (7 - i) * 30;
		td->pos_y = (7 - i) * 20;
	}


	/* Load prefs */
	load_prefs();


	/* Main window (need these before term_getsize gets called) */
	td = &data[0];
	td->dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_SYSMENU |
	               WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_CAPTION |
	               WS_VISIBLE);
	td->dwExStyle = 0;
	td->visible = true;

	/* Sub windows (need these before term_getsize gets called) */
	for (i = 1; i < MAX_TERM_DATA; i++)
	{
		td = &data[i];
		td->dwStyle = (WS_OVERLAPPED | WS_THICKFRAME | WS_SYSMENU);
		td->dwExStyle = (WS_EX_TOOLWINDOW);
	}


	/* All windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		td = &data[i];

		/* Access the standard font file */
		path_build(buf, 1024, ANGBAND_DIR_XTRA_FONT, td->font_want);

		/* Activate the chosen font */
		if (term_force_font(td, buf))
		{
			/* Access the standard font file */
			path_build(buf, 1024, ANGBAND_DIR_XTRA_FONT, "8X13.FON");

			/* Force the use of that font */
			(void)term_force_font(td, buf);

			/* Oops */
			td->tile_wid = 8;
			td->tile_hgt = 13;

			/* Assume not bizarre */
			td->bizarre = false;
		}

		/* Analyze the font */
		term_getsize(td);

		/* Resize the window */
		term_window_resize(td);
	}


	/* Sub windows (reverse order) */
	for (i = MAX_TERM_DATA - 1; i >= 1; --i)
	{
		td = &data[i];

		my_td = td;
		td->w = CreateWindowEx(td->dwExStyle, AngList,
		                       td->s, td->dwStyle,
		                       td->pos_x, td->pos_y,
		                       td->size_wid, td->size_hgt,
		                       HWND_DESKTOP, NULL, hInstance, NULL);
		my_td = NULL;
		if (!td->w) quit("Failed to create sub-window");

		if (td->visible)
		{
			td->size_hack = true;
			ShowWindow(td->w, SW_SHOW);
			td->size_hack = false;
		}

		term_data_link(td);
		angband_term[i] = td->term_ptr;

		if (td->visible)
		{
			/* Activate the window */
			SetActiveWindow(td->w);

			/* Bring window to top */
			SetWindowPos(td->w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
	}


	/* Main window */
	td = &data[0];

	/* Main window */
	my_td = td;
	td->w = CreateWindowEx(td->dwExStyle, AppName,
	                       td->s, td->dwStyle,
	                       td->pos_x, td->pos_y,
	                       td->size_wid, td->size_hgt,
	                       HWND_DESKTOP, NULL, hInstance, NULL);
	my_td = NULL;
	if (!td->w) quit("Failed to create Angband window");

	term_data_link(td);
	angband_term[0] = td->term_ptr;

	/* Activate the main window */
	SetActiveWindow(td->w);

	/* Bring main window back to top */
	SetWindowPos(td->w, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);


	/* New palette XXX XXX XXX */
	(void)new_palette();


	/* Create a "brush" for drawing the "cursor" */
	hbrYellow = CreateSolidBrush(win_clr[TERM_YELLOW]);


	/* Process pending messages */
	(void)Term_xtra_win_flush();
}



/*
 * Prepare the menus
 */
static void setup_menus(void)
{
	int i;

	HMENU hm = GetMenu(data[0].w);


	/* Menu "File", Disable all */
	EnableMenuItem(hm, IDM_FILE_NEW,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_FILE_OPEN,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_FILE_SAVE,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_FILE_SCORE,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_FILE_EXIT,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	/* No character available */
	if (!character_generated)
	{
		/* Menu "File", Item "New" */
		EnableMenuItem(hm, IDM_FILE_NEW,
		               MF_BYCOMMAND | MF_ENABLED);

		/* Menu "File", Item "Open" */
		EnableMenuItem(hm, IDM_FILE_OPEN,
		               MF_BYCOMMAND | MF_ENABLED);
	}

	/* A character available */
	if (character_generated)
	{
		/* Menu "File", Item "Save" */
		EnableMenuItem(hm, IDM_FILE_SAVE,
		               MF_BYCOMMAND | MF_ENABLED);
	}

	/* Menu "File", Item "Score" */
	if (initialized && character_generated && !character_icky)
	{
		EnableMenuItem(hm, IDM_FILE_SCORE,
		               MF_BYCOMMAND | MF_ENABLED);
	}

	/* Menu "File", Item "Exit" */
	EnableMenuItem(hm, IDM_FILE_EXIT,
	               MF_BYCOMMAND | MF_ENABLED);


	/* Menu "Window::Visibility" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		EnableMenuItem(hm, IDM_WINDOW_VIS_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		CheckMenuItem(hm, IDM_WINDOW_VIS_0 + i,
		              (data[i].visible ? MF_CHECKED : MF_UNCHECKED));

		EnableMenuItem(hm, IDM_WINDOW_VIS_0 + i,
		               MF_BYCOMMAND | MF_ENABLED);
	}

	/* Menu "Window::Font" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		EnableMenuItem(hm, IDM_WINDOW_FONT_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible)
		{
			EnableMenuItem(hm, IDM_WINDOW_FONT_0 + i,
			               MF_BYCOMMAND | MF_ENABLED);
		}
	}

	/* Menu "Window::Bizarre Display" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		EnableMenuItem(hm, IDM_WINDOW_BIZ_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		CheckMenuItem(hm, IDM_WINDOW_BIZ_0 + i,
		              (data[i].bizarre ? MF_CHECKED : MF_UNCHECKED));

		if (data[i].visible)
		{
			EnableMenuItem(hm, IDM_WINDOW_BIZ_0 + i,
			               MF_BYCOMMAND | MF_ENABLED);

		}
	}

	/* Menu "Window::Increase Tile Width" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		EnableMenuItem(hm, IDM_WINDOW_I_WID_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible)
		{
			EnableMenuItem(hm, IDM_WINDOW_I_WID_0 + i,
			               MF_BYCOMMAND | MF_ENABLED);

		}
	}

	/* Menu "Window::Decrease Tile Width" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		EnableMenuItem(hm, IDM_WINDOW_D_WID_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible)
		{
			EnableMenuItem(hm, IDM_WINDOW_D_WID_0 + i,
			               MF_BYCOMMAND | MF_ENABLED);

		}
	}

	/* Menu "Window::Increase Tile Height" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		EnableMenuItem(hm, IDM_WINDOW_I_HGT_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible)
		{
			EnableMenuItem(hm, IDM_WINDOW_I_HGT_0 + i,
			               MF_BYCOMMAND | MF_ENABLED);

		}
	}

	/* Menu "Window::Decrease Tile Height" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		EnableMenuItem(hm, IDM_WINDOW_D_HGT_0 + i,
		               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		if (data[i].visible)
		{
			EnableMenuItem(hm, IDM_WINDOW_D_HGT_0 + i,
			               MF_BYCOMMAND | MF_ENABLED);

		}
	}

	/* Menu "Options", disable all */
	EnableMenuItem(hm, IDM_OPTIONS_UNUSED,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(hm, IDM_OPTIONS_SAVER,
	               MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	/* Menu "Options", update all */
	CheckMenuItem(hm, IDM_OPTIONS_UNUSED,
	              (0 ? MF_CHECKED : MF_UNCHECKED));
	CheckMenuItem(hm, IDM_OPTIONS_SAVER,
	              (hwndSaver ? MF_CHECKED : MF_UNCHECKED));



#ifdef USE_SAVER
	/* Menu "Options", Item "ScreenSaver" */
	EnableMenuItem(hm, IDM_OPTIONS_SAVER,
	               MF_BYCOMMAND | MF_ENABLED);
#endif

}


/*
 * Check for double clicked (or dragged) savefile
 *
 * Apparently, Windows copies the entire filename into the first
 * piece of the "command line string".  Perhaps we should extract
 * the "basename" of that filename and append it to the "save" dir.
 */
static void check_for_save_file(LPSTR cmd_line)
{
	char *s, *p;

	/* First arg */
	s = cmd_line;

	/* Second arg */
	p = strchr(s, ' ');

	/* Tokenize, advance */
	if (p) *p++ = '\0';

	/* No args */
	if (!*s) return;

	/* Extract filename */
	strcat(savefile, s);

	/* Validate the file */
	validate_file(savefile);

	/* Game in progress */
	game_in_progress = true;

	/* Play game */
	play_game();
}


/*
 * Process a menu command
 */
static void process_menus(WORD wCmd)
{
	int i;

	term_data *td;

	OPENFILENAME ofn;

	/* Analyze */
	switch (wCmd)
	{
		/* New game */
	case IDM_FILE_NEW:
		{
			if (!initialized)
			{
				plog("You cannot do that yet...");
			}
			else if (game_in_progress)
			{
				plog("You can't start a new game while you're still playing!");
			}
			else
			{
				game_in_progress = true;
				Term_flush();
				play_game();
				quit(NULL);
			}
			break;
		}

		/* Open game */
	case IDM_FILE_OPEN:
		{
			if (!initialized)
			{
				plog("You cannot do that yet...");
			}
			else if (game_in_progress)
			{
				plog("You can't open a new game while you're still playing!");
			}
			else
			{
				memset(&ofn, 0, sizeof(ofn));
#if defined(_WIN32_WINNT) && (_WIN32_WINNT >= 0x0500)
				ofn.lStructSize = sizeof(OPENFILENAME) - (sizeof(void*) + 2 * sizeof(DWORD));
#else // old headers
ofn.lStructSize = sizeof(OPENFILENAME);
#endif
				ofn.hwndOwner = data[0].w;
				ofn.lpstrFilter = "Save Files (*.)\0*\0";
				ofn.nFilterIndex = 1;
				ofn.lpstrFile = savefile;
				ofn.nMaxFile = 1024;
				ofn.lpstrInitialDir = ANGBAND_DIR_SAVE;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

				if (GetOpenFileName(&ofn))
				{
					/* Load 'savefile' */
					validate_file(savefile);
					game_in_progress = true;
					Term_flush();
					play_game();
					quit(NULL);
				}
			}
			break;
		}

		/* Save game */
	case IDM_FILE_SAVE:
		{
			if (game_in_progress && character_generated)
			{
				/* Paranoia */
				if (!inkey_flag)
				{
					plog("You may not do that right now.");
					break;
				}

				/* Hack -- Forget messages */
				msg_flag = false;

				/* Save the game */
				do_cmd_save_game();
			}
			else
			{
				plog("You may not do that right now.");
			}
			break;
		}

		/* Exit */
	case IDM_FILE_EXIT:
		{
			if (game_in_progress && character_generated)
			{
				/* Paranoia */
				if (!inkey_flag)
				{
					plog("You may not do that right now.");
					break;
				}

				/* Hack -- Forget messages */
				msg_flag = false;

				/* Save the game */
				do_cmd_save_game();
			}
			quit(NULL);
			break;
		}

		/* Score */
	case IDM_FILE_SCORE:
		{
			predict_score_gui(&initialized, &game_in_progress);

			/* Done */
			break;
		}

	case IDM_WINDOW_VIS_0:
		{
			plog("You are not allowed to do that!");

			break;
		}

		/* Window visibility */
	case IDM_WINDOW_VIS_1:
	case IDM_WINDOW_VIS_2:
	case IDM_WINDOW_VIS_3:
	case IDM_WINDOW_VIS_4:
	case IDM_WINDOW_VIS_5:
	case IDM_WINDOW_VIS_6:
	case IDM_WINDOW_VIS_7:
		{
			i = wCmd - IDM_WINDOW_VIS_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			if (!td->visible)
			{
				td->visible = true;
				ShowWindow(td->w, SW_SHOW);
				term_data_redraw(td);
			}
			else
			{
				td->visible = false;
				ShowWindow(td->w, SW_HIDE);
			}

			break;
		}

		/* Window fonts */
	case IDM_WINDOW_FONT_0:
	case IDM_WINDOW_FONT_1:
	case IDM_WINDOW_FONT_2:
	case IDM_WINDOW_FONT_3:
	case IDM_WINDOW_FONT_4:
	case IDM_WINDOW_FONT_5:
	case IDM_WINDOW_FONT_6:
	case IDM_WINDOW_FONT_7:
		{
			i = wCmd - IDM_WINDOW_FONT_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			term_change_font(td);

			break;
		}

		/* Bizarre Display */
	case IDM_WINDOW_BIZ_0:
	case IDM_WINDOW_BIZ_1:
	case IDM_WINDOW_BIZ_2:
	case IDM_WINDOW_BIZ_3:
	case IDM_WINDOW_BIZ_4:
	case IDM_WINDOW_BIZ_5:
	case IDM_WINDOW_BIZ_6:
	case IDM_WINDOW_BIZ_7:
		{
			i = wCmd - IDM_WINDOW_BIZ_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			td->bizarre = !td->bizarre;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		/* Increase Tile Width */
	case IDM_WINDOW_I_WID_0:
	case IDM_WINDOW_I_WID_1:
	case IDM_WINDOW_I_WID_2:
	case IDM_WINDOW_I_WID_3:
	case IDM_WINDOW_I_WID_4:
	case IDM_WINDOW_I_WID_5:
	case IDM_WINDOW_I_WID_6:
	case IDM_WINDOW_I_WID_7:
		{
			i = wCmd - IDM_WINDOW_I_WID_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			td->tile_wid += 1;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		/* Decrease Tile Height */
	case IDM_WINDOW_D_WID_0:
	case IDM_WINDOW_D_WID_1:
	case IDM_WINDOW_D_WID_2:
	case IDM_WINDOW_D_WID_3:
	case IDM_WINDOW_D_WID_4:
	case IDM_WINDOW_D_WID_5:
	case IDM_WINDOW_D_WID_6:
	case IDM_WINDOW_D_WID_7:
		{
			i = wCmd - IDM_WINDOW_D_WID_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			td->tile_wid -= 1;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		/* Increase Tile Height */
	case IDM_WINDOW_I_HGT_0:
	case IDM_WINDOW_I_HGT_1:
	case IDM_WINDOW_I_HGT_2:
	case IDM_WINDOW_I_HGT_3:
	case IDM_WINDOW_I_HGT_4:
	case IDM_WINDOW_I_HGT_5:
	case IDM_WINDOW_I_HGT_6:
	case IDM_WINDOW_I_HGT_7:
		{
			i = wCmd - IDM_WINDOW_I_HGT_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			td->tile_hgt += 1;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

		/* Decrease Tile Height */
	case IDM_WINDOW_D_HGT_0:
	case IDM_WINDOW_D_HGT_1:
	case IDM_WINDOW_D_HGT_2:
	case IDM_WINDOW_D_HGT_3:
	case IDM_WINDOW_D_HGT_4:
	case IDM_WINDOW_D_HGT_5:
	case IDM_WINDOW_D_HGT_6:
	case IDM_WINDOW_D_HGT_7:
		{
			i = wCmd - IDM_WINDOW_D_HGT_0;

			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			td = &data[i];

			td->tile_hgt -= 1;

			term_getsize(td);

			term_window_resize(td);

			break;
		}

	case IDM_OPTIONS_UNUSED:
		{
			/* Unused for now XXX XXX XXX */

			break;
		}

#ifdef USE_SAVER

	case IDM_OPTIONS_SAVER:
		{
			if (hwndSaver)
			{
				DestroyWindow(hwndSaver);
				hwndSaver = NULL;
			}
			else
			{
				/* Create a screen scaver window */
				hwndSaver = CreateWindowEx(WS_EX_TOPMOST, "WindowsScreenSaverClass",
				                           "Angband Screensaver",
				                           WS_POPUP | WS_MAXIMIZE | WS_VISIBLE,
				                           0, 0, GetSystemMetrics(SM_CXSCREEN),
				                           GetSystemMetrics(SM_CYSCREEN),
				                           NULL, NULL, hInstance, NULL);

				if (hwndSaver)
				{
					/* Push the window to the bottom XXX XXX XXX */
					SetWindowPos(hwndSaver, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				}
				else
				{
					plog("Failed to create saver window");
				}
			}
			break;
		}

#endif

}



LRESULT FAR PASCAL AngbandWndProc(HWND hWnd, UINT uMsg,
                                  WPARAM wParam, LPARAM lParam)
{
	PAINTSTRUCT ps;
	HDC hdc;
	term_data *td;
	MINMAXINFO FAR *lpmmi;
	RECT rc;
	int i;


	/* Acquire proper "term_data" info */
	td = (term_data *)GetWindowLong(hWnd, 0);

	/* Handle message */
	switch (uMsg)
	{
		/* XXX XXX XXX */
	case WM_NCCREATE:
		{
			SetWindowLong(hWnd, 0, (LONG)(my_td));
			break;
		}

		/* XXX XXX XXX */
	case WM_CREATE:
		{
			return 0;
		}

	case WM_GETMINMAXINFO:
		{
			lpmmi = (MINMAXINFO FAR *)lParam;

			/* this message was sent before WM_NCCREATE */
			if (!td) return 1;

			/* Minimum window size is 8x2 */
			rc.left = rc.top = 0;
			rc.right = rc.left + 8 * td->tile_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 2 * td->tile_hgt + td->size_oh1 + td->size_oh2 + 1;

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, true, td->dwExStyle);

			/* Save minimum size */
			lpmmi->ptMinTrackSize.x = rc.right - rc.left;
			lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

			/* Maximum window size */
			rc.left = rc.top = 0;
			rc.right = rc.left + 255 * td->tile_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 255 * td->tile_hgt + td->size_oh1 + td->size_oh2;

			/* Paranoia */
			rc.right += (td->tile_wid - 1);
			rc.bottom += (td->tile_hgt - 1);

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, true, td->dwExStyle);

			/* Save maximum size */
			lpmmi->ptMaxSize.x = rc.right - rc.left;
			lpmmi->ptMaxSize.y = rc.bottom - rc.top;

			/* Save maximum size */
			lpmmi->ptMaxTrackSize.x = rc.right - rc.left;
			lpmmi->ptMaxTrackSize.y = rc.bottom - rc.top;

			return 0;
		}

	case WM_PAINT:
		{
			BeginPaint(hWnd, &ps);
			if (td) term_data_redraw(td);
			EndPaint(hWnd, &ps);
			ValidateRect(hWnd, NULL);
			return 0;
		}

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		{
			/* Unused */
			/* BYTE KeyState = 0x00; */

			bool mc = false;
			bool ms = false;
			bool ma = false;

			/* Extract the modifiers */
			if (GetKeyState(VK_CONTROL) & 0x8000) mc = true;
			if (GetKeyState(VK_SHIFT) & 0x8000) ms = true;
			if (GetKeyState(VK_MENU) & 0x8000) ma = true;

			/* Handle "special" keys */
			if (special_key[(byte)(wParam)] || (ma && !ignore_key[(byte)(wParam)]) )
			{
				/* Begin the macro trigger */
				Term_keypress(31);

				/* Send the modifiers */
				if (mc) Term_keypress('C');
				if (ms) Term_keypress('S');
				if (ma) Term_keypress('A');

				/* Extract "scan code" */
				i = LOBYTE(HIWORD(lParam));

				/* Introduce the scan code */
				Term_keypress('x');

				/* Encode the hexidecimal scan code */
				Term_keypress(hexsym[i / 16]);
				Term_keypress(hexsym[i % 16]);

				/* End the macro trigger */
				Term_keypress(13);

				return 0;
			}

			break;
		}

	case WM_CHAR:
		{
			Term_keypress(wParam);
			return 0;
		}

	case WM_INITMENU:
		{
			setup_menus();
			return 0;
		}

	case WM_CLOSE:
		{
			if (game_in_progress && character_generated)
			{
				/* Hack -- Forget messages */
				msg_flag = false;

				/* Save the game */
				do_cmd_save_game();
			}
			quit(NULL);
			return 0;
		}

	case WM_QUIT:
		{
			quit(NULL);
			return 0;
		}

	case WM_COMMAND:
		{
			process_menus(LOWORD(wParam));
			return 0;
		}

	case WM_SIZE:
		{
			/* this message was sent before WM_NCCREATE */
			if (!td) return 1;

			/* it was sent from inside CreateWindowEx */
			if (!td->w) return 1;

			/* was sent from WM_SIZE */
			if (td->size_hack) return 1;

			switch (wParam)
			{
			case SIZE_MINIMIZED:
				{
					/* Hide sub-windows */
					for (i = 1; i < MAX_TERM_DATA; i++)
					{
						if (data[i].visible) ShowWindow(data[i].w, SW_HIDE);
					}
					return 0;
				}

			case SIZE_MAXIMIZED:
				{
					/* fall through XXX XXX XXX */
				}

			case SIZE_RESTORED:
				{
					unsigned int cols = (LOWORD(lParam) - td->size_ow1 - td->size_ow2) / td->tile_wid;
					unsigned int rows = (HIWORD(lParam) - td->size_oh1 - td->size_oh2) / td->tile_hgt;

					/* New size */
					if ((td->cols != cols) || (td->rows != rows))
					{
						/* save the new size */
						td->cols = cols;
						td->rows = rows;

						/* Activate */
						Term_activate(td->term_ptr);

						/* Resize the term */
						Term_resize(td->cols, td->rows);

						/* Redraw later */
						InvalidateRect(td->w, NULL, true);
					}

					td->size_hack = true;

					/* Restore sub-windows */
					for (i = 1; i < MAX_TERM_DATA; i++)
					{
						if (data[i].visible) ShowWindow(data[i].w, SW_SHOWNOACTIVATE);
					}

					td->size_hack = false;

					return 0;
				}
			}
			break;
		}

	case WM_PALETTECHANGED:
		{
			/* Ignore if palette change caused by itself */
			if ((HWND)wParam == hWnd) return 0;

			/* Fall through... */
		}

	case WM_QUERYNEWPALETTE:
		{
			if (!paletted) return 0;

			hdc = GetDC(hWnd);

			SelectPalette(hdc, hPal, false);

			i = RealizePalette(hdc);

			/* if any palette entries changed, repaint the window. */
			if (i) InvalidateRect(hWnd, NULL, true);

			ReleaseDC(hWnd, hdc);

			return 0;
		}

	case WM_ACTIVATE:
		{
			if (wParam && !HIWORD(lParam))
			{
				/* Do something to sub-windows */
				for (i = 1; i < MAX_TERM_DATA; i++)
				{
					SetWindowPos(data[i].w, hWnd, 0, 0, 0, 0,
					             SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
				}

				/* Focus on main window */
				SetFocus(hWnd);

				return 0;
			}

			break;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


LRESULT FAR PASCAL AngbandListProc(HWND hWnd, UINT uMsg,
                                   WPARAM wParam, LPARAM lParam)
{
	term_data *td;
	MINMAXINFO FAR *lpmmi;
	RECT rc;
	PAINTSTRUCT ps;
	HDC hdc;
	int i;


	/* Acquire proper "term_data" info */
	td = (term_data *)GetWindowLong(hWnd, 0);

	/* Process message */
	switch (uMsg)
	{
		/* XXX XXX XXX */
	case WM_NCCREATE:
		{
			SetWindowLong(hWnd, 0, (LONG)(my_td));
			break;
		}

		/* XXX XXX XXX */
	case WM_CREATE:
		{
			return 0;
		}

	case WM_GETMINMAXINFO:
		{
			/* this message was sent before WM_NCCREATE */
			if (!td) return 1;

			lpmmi = (MINMAXINFO FAR *)lParam;

			/* Minimum size */
			rc.left = rc.top = 0;
			rc.right = rc.left + 8 * td->tile_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 2 * td->tile_hgt + td->size_oh1 + td->size_oh2;

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, true, td->dwExStyle);

			/* Save the minimum size */
			lpmmi->ptMinTrackSize.x = rc.right - rc.left;
			lpmmi->ptMinTrackSize.y = rc.bottom - rc.top;

			/* Maximum window size */
			rc.left = rc.top = 0;
			rc.right = rc.left + 80 * td->tile_wid + td->size_ow1 + td->size_ow2;
			rc.bottom = rc.top + 24 * td->tile_hgt + td->size_oh1 + td->size_oh2;

			/* Paranoia */
			rc.right += (td->tile_wid - 1);
			rc.bottom += (td->tile_hgt - 1);

			/* Adjust */
			AdjustWindowRectEx(&rc, td->dwStyle, true, td->dwExStyle);

			/* Save maximum size */
			lpmmi->ptMaxSize.x = rc.right - rc.left;
			lpmmi->ptMaxSize.y = rc.bottom - rc.top;

			/* Save the maximum size */
			lpmmi->ptMaxTrackSize.x = rc.right - rc.left;
			lpmmi->ptMaxTrackSize.y = rc.bottom - rc.top;

			return 0;
		}

	case WM_SIZE:
		{
			/* this message was sent before WM_NCCREATE */
			if (!td) return 1;

			/* it was sent from inside CreateWindowEx */
			if (!td->w) return 1;

			/* was sent from inside WM_SIZE */
			if (td->size_hack) return 1;

			td->size_hack = true;

			td->cols = (LOWORD(lParam) - td->size_ow1 - td->size_ow2) / td->tile_wid;
			td->rows = (HIWORD(lParam) - td->size_oh1 - td->size_oh2) / td->tile_hgt;

			term_getsize(td);

			MoveWindow(hWnd, td->pos_x, td->pos_y, td->size_wid, td->size_hgt, true);

			td->size_hack = false;

			return 0;
		}

	case WM_PAINT:
		{
			BeginPaint(hWnd, &ps);
			if (td) term_data_redraw(td);
			EndPaint(hWnd, &ps);
			return 0;
		}

	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		{
			/* Unused */
			/* BYTE KeyState = 0x00; */

			bool mc = false;
			bool ms = false;
			bool ma = false;

			/* Extract the modifiers */
			if (GetKeyState(VK_CONTROL) & 0x8000) mc = true;
			if (GetKeyState(VK_SHIFT) & 0x8000) ms = true;
			if (GetKeyState(VK_MENU) & 0x8000) ma = true;

			/* Handle "special" keys */
			if (special_key[(byte)(wParam)] || (ma && !ignore_key[(byte)(wParam)]) )
			{
				/* Begin the macro trigger */
				Term_keypress(31);

				/* Send the modifiers */
				if (mc) Term_keypress('C');
				if (ms) Term_keypress('S');
				if (ma) Term_keypress('A');

				/* Extract "scan code" */
				i = LOBYTE(HIWORD(lParam));

				/* Introduce the scan code */
				Term_keypress('x');

				/* Encode the hexidecimal scan code */
				Term_keypress(hexsym[i / 16]);
				Term_keypress(hexsym[i % 16]);

				/* End the macro trigger */
				Term_keypress(13);

				return 0;
			}

			break;
		}

	case WM_CHAR:
		{
			Term_keypress(wParam);
			return 0;
		}

	case WM_PALETTECHANGED:
		{
			/* ignore if palette change caused by itself */
			if ((HWND)wParam == hWnd) return false;
			/* otherwise, fall through!!! */
		}

	case WM_QUERYNEWPALETTE:
		{
			if (!paletted) return 0;
			hdc = GetDC(hWnd);
			SelectPalette(hdc, hPal, false);
			i = RealizePalette(hdc);
			/* if any palette entries changed, repaint the window. */
			if (i) InvalidateRect(hWnd, NULL, true);
			ReleaseDC(hWnd, hdc);
			return 0;
		}

	case WM_NCLBUTTONDOWN:
		{

#ifdef HTCLOSE
			if (wParam == HTCLOSE) wParam = HTSYSMENU;
#endif /* HTCLOSE */

			if (wParam == HTSYSMENU)
			{
				if (td->visible)
				{
					td->visible = false;
					ShowWindow(td->w, SW_HIDE);
				}

				return 0;
			}

			break;
		}
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


#ifdef USE_SAVER

#define MOUSE_SENS 40

LRESULT FAR PASCAL AngbandSaverProc(HWND hWnd, UINT uMsg,
WPARAM wParam, LPARAM lParam)
{
	static int iMouse = 0;
	static WORD xMouse = 0;
	static WORD yMouse = 0;

	int dx, dy;


	/* Process */
	switch (uMsg)
	{
		/* XXX XXX XXX */
	case WM_NCCREATE:
		{
			break;
		}

	case WM_SETCURSOR:
		{
			SetCursor(NULL);
			return 0;
		}

	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_KEYDOWN:
		{
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			return 0;
		}

	case WM_MOUSEMOVE:
		{
			if (iMouse)
			{
				dx = LOWORD(lParam) - xMouse;
				dy = HIWORD(lParam) - yMouse;

				if (dx < 0) dx = -dx;
				if (dy < 0) dy = -dy;

				if ((dx > MOUSE_SENS) || (dy > MOUSE_SENS))
				{
					SendMessage(hWnd, WM_CLOSE, 0, 0);
				}
			}

			/* Save last location */
			iMouse = 1;
			xMouse = LOWORD(lParam);
			yMouse = HIWORD(lParam);

			return 0;
		}

	case WM_CLOSE:
		{
			DestroyWindow(hwndSaver);
			hwndSaver = NULL;
			return 0;
		}
	}

	/* Oops */
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

#endif /* USE_SAVER */





/*** Temporary Hooks ***/


/*
 * Display warning message (see "z-util.c")
 */
static void hack_plog(const char *str)
{
	/* Give a warning */
	if (str)
	{
		MessageBox(NULL, str, "Warning",
		           MB_ICONEXCLAMATION | MB_OK);
	}
}


/*
 * Display error message and quit (see "z-util.c")
 */
static void hack_quit(const char *str)
{
	/* Give a warning */
	if (str)
	{
		MessageBox(NULL, str, "Error",
		           MB_ICONEXCLAMATION | MB_OK | MB_ICONSTOP);
	}

	/* Unregister the classes */
	UnregisterClass(AppName, hInstance);

	/* Destroy the icon */
	if (hIcon) DestroyIcon(hIcon);

	/* Exit */
	exit(0);
}



/*** Various hooks ***/


/*
 * Display warning message (see "z-util.c")
 */
static void hook_plog(const char *str)
{
	/* Warning */
	if (str)
	{
		MessageBox(data[0].w, str, "Warning",
		           MB_ICONEXCLAMATION | MB_OK);
	}
}


/*
 * Display error message and quit (see "z-util.c")
 */
static void hook_quit(const char *str)
{
	int i;


	/* Give a warning */
	if (str)
	{
		MessageBox(data[0].w, str, "Error",
		           MB_ICONEXCLAMATION | MB_OK | MB_ICONSTOP);
	}


	/* Save the preferences */
	save_prefs();


	/*** Could use 'Term_nuke_win()' XXX XXX XXX */

	/* Destroy all windows */
	for (i = MAX_TERM_DATA - 1; i >= 0; --i)
	{
		term_force_font(&data[i], NULL);
		if (data[i].font_want)
		{
			free(data[i].font_want);
			data[i].font_want = NULL;
		}
		if (data[i].w) DestroyWindow(data[i].w);
		data[i].w = 0;
	}


	/*** Free some other stuff ***/

	DeleteObject(hbrYellow);

	if (hPal) DeleteObject(hPal);

	UnregisterClass(AppName, hInstance);

	if (hIcon) DestroyIcon(hIcon);

	exit(0);
}

/*** Initialize ***/


/*
 * Init some stuff
 */
static void init_stuff(void)
{
	int i;

	char path[1024];


	/* Get program name with full path */
	GetModuleFileName(hInstance, path, 512);

	/* Get the name of the "*.ini" file */
	strcpy(path + strlen(path) - 4, ".INI");

	/* Save the the name of the ini-file */
	ini_file = strdup(path);

	/* Validate the ini-file */
	validate_file(ini_file);

	/* Analyze the path */
	i = strlen(path);

	/* Get the path */
	for (; i > 0; i--)
	{
		if (path[i] == '\\')
		{
			/* End of path */
			break;
		}
	}

	/* Add "lib" to the path */
	strcpy(path + i + 1, "lib\\");

	/* Validate the path */
	validate_dir(path);

	/*** Initialise the file paths ***/

	/* Start with standard ones */
	init_file_paths(path);

	/* Build the "font" path */
	path_build(path, 1024, ANGBAND_DIR_XTRA, "font");

	/* Allocate the path */
	ANGBAND_DIR_XTRA_FONT = strdup(path);


	/*** Validate the paths to ensure we have a working install ***/

	validate_dir(ANGBAND_DIR_DATA);
	validate_dir(ANGBAND_DIR_EDIT);
	validate_dir(ANGBAND_DIR_FILE);
	validate_dir(ANGBAND_DIR_HELP);
	validate_dir(ANGBAND_DIR_INFO);
	validate_dir(ANGBAND_DIR_NOTE);
	validate_dir(ANGBAND_DIR_SAVE);
	validate_dir(ANGBAND_DIR_PREF);
	validate_dir(ANGBAND_DIR_USER);
	validate_dir(ANGBAND_DIR_XTRA);
	validate_dir(ANGBAND_DIR_CMOV);
	validate_dir(ANGBAND_DIR_XTRA_FONT);

	/* Build the filename */
	path_build(path, 1024, ANGBAND_DIR_XTRA_FONT, "8X13.FON");

	/* Hack -- Validate the basic font */
	validate_file(path);






	/* Build the "help" path */
	path_build(path, 1024, ANGBAND_DIR_XTRA, "help");

	/* Allocate the path */
	ANGBAND_DIR_XTRA_HELP = strdup(path);

	/* Validate the "help" directory */
	/* validate_dir(ANGBAND_DIR_XTRA_HELP); */
}

int FAR PASCAL WinMain(HINSTANCE hInst, HINSTANCE hPrevInst,
                       LPSTR lpCmdLine, int nCmdShow)
{
	int i;

	WNDCLASS wc;
	HDC hdc;
	MSG msg;

	/* Save globally */
	hInstance = hInst;

	/* Initialize */
	if (hPrevInst == NULL)
	{
		wc.style = CS_CLASSDC;
		wc.lpfnWndProc = AngbandWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 4;  /* one long pointer to term_data */
		wc.hInstance = hInst;
		wc.hIcon = hIcon = LoadIcon(hInst, AppName);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = GetStockObject(BLACK_BRUSH);
		wc.lpszMenuName = AppName;
		wc.lpszClassName = AppName;

		if (!RegisterClass(&wc)) exit(1);

		wc.lpfnWndProc = AngbandListProc;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = AngList;

		if (!RegisterClass(&wc)) exit(2);

#ifdef USE_SAVER

		wc.style = CS_VREDRAW | CS_HREDRAW | CS_SAVEBITS | CS_DBLCLKS;
		wc.lpfnWndProc = AngbandSaverProc;
		wc.hCursor = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = "WindowsScreenSaverClass";

		if (!RegisterClass(&wc)) exit(3);

#endif

	}

	/* Temporary hooks */
	plog_aux = hack_plog;
	quit_aux = hack_quit;

	/* Prepare the filepaths */
	init_stuff();

	/* Initialize the keypress analyzer */
	for (i = 0; special_key_list[i]; ++i)
	{
		special_key[special_key_list[i]] = true;
	}

	/* Determine if display is 16/256/true color */
	hdc = GetDC(NULL);
	colors16 = (GetDeviceCaps(hdc, BITSPIXEL) == 4);
	paletted = ((GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) ? true : false);
	ReleaseDC(NULL, hdc);

	/* Initialize the colors */
	for (i = 0; i < 256; i++)
	{
		byte rv, gv, bv;

		/* Extract desired values */
		rv = angband_color_table[i][1];
		gv = angband_color_table[i][2];
		bv = angband_color_table[i][3];

		/* Extract the "complex" code */
		win_clr[i] = PALETTERGB(rv, gv, bv);

		/* Save the "simple" code */
		angband_color_table[i][0] = win_pal[i];
	}

	/* Prepare the windows */
	init_windows();

	/* Activate hooks */
	plog_aux = hook_plog;
	quit_aux = hook_quit;
	core_aux = hook_quit;

	/* Set the system suffix */
	ANGBAND_SYS = "win";


	/* Set the keyboard suffix */
	if (7 != GetKeyboardType(0))
		ANGBAND_KEYBOARD = "0";
	else
	{
		/* Japanese keyboard */
		switch (GetKeyboardType(1))
		{
		case 0x0D01:
		case 0x0D02:
		case 0x0D03:
		case 0x0D04:
		case 0x0D05:
		case 0x0D06:
			/* NEC PC-98x1 */
			ANGBAND_KEYBOARD = "NEC98";
			break;
		default:
			/* PC/AT */
			ANGBAND_KEYBOARD = "JAPAN";
		}
	}

	/* Initialize */
	init_angband();

	/* Prompt the user */
	prt("", 23, 0);
	prt("[Press any key to proceed]", 23, 27);
	Term_fresh();

	inkey();

	/* We are now initialized */
	initialized = true;

	/* Did the user double click on a save file? */
	check_for_save_file(lpCmdLine);

	game_in_progress = true;
	play_game();

	/* Prompt the user */
	Term_fresh();

	/* Process messages forever */
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	/* Initialize the keypress analyzer */
	for (i = 0; ignore_key_list[i]; ++i)
	{
		ignore_key[ignore_key_list[i]] = true;
	}

	/* Paranoia */
	quit(NULL);

	/* Paranoia */
	return (0);
}

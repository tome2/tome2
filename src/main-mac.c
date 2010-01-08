/* File: main-mac.c */

/*
 * Copyright (c) 1997 Ben Harrison, Keith Randall, and others
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.
 */


/*
 * This file helps Angband work with Macintosh computers.
 *
 * To use this file, use an appropriate "Makefile" or "Project File", which
 * should define "MACINTOSH".
 *
 * The official compilation uses the CodeWarrior Pro compiler.
 *
 * If you are never going to use "graphics" (especially if you are not
 * compiling support for graphics anyway) then you can delete the "pict"
 * resource with id "1001" with no dangerous side effects.
 *
 *
 * By default, this file assumes that you will be using a 68020 or better
 * machine, running System 7 and Color Quickdraw.  In fact, the game will
 * refuse to run unless these features are available.  This allows the use
 * of a variety of interesting features such as graphics and sound.
 *
 * To create a version which can be used on 68000 machines, or on machines
 * which are not running System 7 or Color Quickdraw, simply activate the
 * "ANGBAND_LITE_MAC" compilation flag in the proper header file.  This
 * will disable all "modern" features used in this file, including support
 * for multiple sub-windows, color, graphics, and sound.
 *
 * When compiling with the "ANGBAND_LITE_MAC" flag, the "ANGBAND_LITE"
 * flag will be automatically defined, which will disable many of the
 * advanced features of the game itself, reducing the total memory usage.
 *
 *
 * Note that the "preference" file is now a simple text file called
 * "Angband Preferences", which contains a version stamp, so that
 * obsolete preference files can be ignored.  This should probably
 * be replaced with a "structured" preference file of some kind.
 *
 * Note that "init1.c", "init2.c", "load1.c", "load2.c", and "birth.c"
 * should probably be "unloaded" as soon as they are no longer needed,
 * to save space, but I do not know how to do this.  XXX XXX XXX
 *
 * Stange bug -- The first "ClipRect()" call crashes if the user closes
 * all the windows, switches to another application, switches back, and
 * re-opens the main window, for example, using "command-a".  XXX XXX XXX
 *
 *
 * Initial framework (and most code) by Ben Harrison (benh@phial.com).
 *
 * Some code adapted from "MacAngband 2.6.1" by Keith Randall
 *
 * Initial PowerMac port by Maarten Hazewinkel (mmhazewi@cs.ruu.nl).
 *
 * Most "USE_SFL_CODE" code provided by Steve Linberg (slinberg@crocker.com).
 *
 * Most of the graphics code is adapted from an extremely minimal subset of
 * the "Sprite World II" package, an amazing (and free) animation package.
 *
 *
 * Important Resources in the resource file:
 *
 *   FREF 130 = ANGBAND_CREATOR / 'APPL' (application)
 *   FREF 129 = ANGBAND_CREATOR / 'SAVE' (save file)
 *   FREF 130 = ANGBAND_CREATOR / 'TEXT' (bone file, generic text file)
 *   FREF 131 = ANGBAND_CREATOR / 'DATA' (binary image file, score file)
 *
 *   DLOG 128 = "About Angband..."
 *
 *   ALRT 128 = unused (?)
 *   ALRT 129 = "Warning..."
 *   ALRT 130 = "Are you sure you want to quit without saving?"
 *
 *   DITL 128 = body for DLOG 128
 *   DITL 129 = body for ALRT 129
 *   DITL 130 = body for ALRT 130
 *
 *   ICON 128 = "warning" icon
 *
 *   MENU 128 = apple (about, -, ...)
 *   MENU 129 = File (new, open, close, save, -, score, exit, quit)
 *     In T.o.M.E.
 *   MENU 129 = File (close, save, -, score, exit, quit)
 *   MENU 130 = Edit (undo, -, cut, copy, paste, clear)
 *   MENU 131 = Font (bold, wide, -)
 *   MENU 132 = Size ()
 *   MENU 133 = Windows ()
 *   MENU 134 = Special (Sound, Graphics, TileWidth, TileHeight, -, Fiddle, Wizard)
 *              Graphics have following submenu attached:
 *   MENU 144 = Graphics (None, 8x8, 16x16)
 *              TileWidth and TileHeight submenus are filled in by this program
 *   MENU 145 = TileWidth ()
 *   MENU 146 = TileHeight ()
 *
 *   PICT 1001 = Graphics tile set (8x8)
 *   PICT 1002 = Graphics tile set (16x16 images)
 *
 * Note: You can no longer use the exit menu unless you build the programme
 *       with an appropriate compile-time option.
 *
 *
 * File name patterns:
 *   all 'APEX' files have a filename of the form "*:apex:*" (?)
 *   all 'BONE' files have a filename of the form "*:bone:*" (?)
 *   all 'DATA' files have a filename of the form "*:data:*"
 *   all 'SAVE' files have a filename of the form "*:save:*"
 *   all 'USER' files have a filename of the form "*:user:*" (?)
 *
 * Perhaps we should attempt to set the "_ftype" flag inside this file,
 * to avoid nasty file type information being spread all through the
 * rest of the code.  (?)  This might require adding hooks into the
 * "fd_open()" and "my_fopen()" functions in "util.c".  XXX XXX XXX
 *
 *
 * Reasons for each header file:
 *
 *   angband.h = Angband header file
 *
 *   Types.h = (included anyway)
 *   Gestalt.h = gestalt code
 *   QuickDraw.h = (included anyway)
 *   OSUtils.h = (included anyway)
 *   Files.h = file code
 *   Fonts.h = font code
 *   Menus.h = menu code
 *   Dialogs.h = dialog code
 *   Windows.h = (included anyway)
 *   Palettes.h = palette code
 *   StandardFile.h = file dialog box
 *   DiskInit.h = disk initialization
 *   ToolUtils.h = HiWord() / LoWord()
 *   Desk.h = OpenDeskAcc()
 *   Devices.h = OpenDeskAcc()
 *   Events.h = event code
 *   Resources.h = resource code
 *   Controls.h = button code
 *   SegLoad.h = ExitToShell(), AppFile, etc
 *   Memory.h = SetApplLimit(), NewPtr(), etc
 *   QDOffscreen.h = GWorld code
 *   Sound.h = Sound code
 *
 * For backwards compatibility:
 *   Use GestaltEqu.h instead of Gestalt.h
 *   Add Desk.h to include simply includes Menus.h, Devices.h, Events.h
 */


#include "angband.h"


#ifdef MACINTOSH

/*
 * Variant-dependent features:
 *
 * #define ALLOW_BIG_SCREEN (V, Ey, O, T.o.M.E. and Z.
 *   Dr's big screen needs more work.  New S one needs some thought)
 * #define ANG281_RESET_VISUALS (Cth, Gum, T.o.M.E., Z)
 * #define SAVEFILE_SCREEN (T.o.M.E.)
 * #define USE_DOUBLE_TILES ("bigtile" patch, V and T.o.M.E.
 *   T requires #define TOME in addition to this)
 * #define ZANG_AUTO_SAVE (O and Z)
 * #define HAS_SCORE_MENU (V and T.o.M.E.)
 * #define ANGBAND_PREFERENCES "_your_variant_name_ Preferences"
 * #define ANGBAND_CREATOR four letter code for your variant, if any.
 *  or use the default one.
 *
 * In [Z], please replace inkey_flag with p_ptr->inkey_flag as well.
 */

/* Some porting examples */
#ifdef ANGBAND30X
# define USE_DOUBLE_TILES
# define ALLOW_BIG_SCREEN
# define HAS_SCORE_MENU
# define NEW_ZVIRT_HOOKS
/* I can't ditch this, yet, because there are many variants */
# define USE_TRANSPARENCY
#endif /* ANGBAND30X */

#ifdef TOME
# define USE_DOUBLE_TILES
# define SAVEFILE_SCREEN
# define ANG281_RESET_VISUALS
# define ALLOW_BIG_SCREEN
# define HAS_SCORE_MENU
# define ANGBAND_CREATOR 'PrnA'
# define ANGBAND_PREFERENCES "T.o.M.E. Preferences"
#endif /* TOME */

/* Default creator signature */
# ifndef ANGBAND_CREATOR
# define ANGBAND_CREATOR 'A271'
# endif 

/* Default preferences file name */
# ifndef ANGBAND_PREFERENCES
# define ANGBAND_PREFERENCES "Angband Preferences"
# endif 


/*
 * To cope with pref file related problems
 *
 * Please note that some variants don't set them to "real" version number
 * and uses other defines for that purpose.
 *
 * This is *very* important for Classic ports, because wrong pref file formats
 * can crash the system.
 */
#ifndef PREF_VER_MAJOR
# define PREF_VER_MAJOR VERSION_MAJOR
#endif
#ifndef PREF_VER_MINOR
# define PREF_VER_MINOR VERSION_MINOR
#endif
#ifndef PREF_VER_PATCH
# define PREF_VER_PATCH VERSION_PATCH
#endif
#ifndef PREF_VER_EXTRA
# define PREF_VER_EXTRA VERSION_EXTRA
#endif


#include <Types.h>
#include <Gestalt.h>
#include <QuickDraw.h>
#include <Files.h>
#include <Fonts.h>
#include <Menus.h>
#include <Dialogs.h>
#include <Windows.h>
#include <Palettes.h>
#include <StandardFile.h>
#include <DiskInit.h>
#include <ToolUtils.h>
#include <Devices.h>
#include <Events.h>
#include <Resources.h>
#include <Controls.h>
#include <SegLoad.h>
#include <Memory.h>
#include <QDOffscreen.h>
#include <Sound.h>


/*
 * Use "malloc()" instead of "NewPtr()"
 */
/* #define USE_MALLOC */


#if defined(powerc) || defined(__powerc)

/*
 * Disable "LITE" version
 */
# undef ANGBAND_LITE_MAC

#endif


#ifndef ANGBAND_LITE_MAC

/*
 * Activate some special code
 */
# define USE_SFL_CODE

#endif /* ANGBAND_LITE_MAC */



#ifdef USE_SFL_CODE

/*
 * Include the necessary header files
 */
#include <AppleEvents.h>
#include <EPPC.h>
#include <Folders.h>

#endif


#ifdef ANGBAND_LITE_MAC

/*
 * Everything in drawn as white on black
 */

#else /* ANGBAND_LITE_MAC */

/*
* Information about each of the 256 available colors
*/
static RGBColor color_info[256];

#endif /* ANGBAND_LITE_MAC */


/*
 * Forward declare
 */
typedef struct term_data term_data;

/*
 * Extra "term" data
 */
struct term_data
{
	term *t;

	Rect r;

	WindowPtr w;

#ifdef ANGBAND_LITE_MAC

	/* Nothing */

#else /* ANGBAND_LITE_MAC */

	short padding;

	short pixelDepth;

	GWorldPtr theGWorld;

	GDHandle theGDH;

	GDHandle mainSWGDH;

#endif /* ANGBAND_LITE_MAC */

	Str15 title;

	s16b oops;

	s16b keys;

	s16b last;

	s16b mapped;

	s16b rows;
	s16b cols;

	s16b font_id;
	s16b font_size;
	s16b font_face;
	s16b font_mono;

	s16b font_o_x;
	s16b font_o_y;
	s16b font_wid;
	s16b font_hgt;

	s16b tile_o_x;
	s16b tile_o_y;
	s16b tile_wid;
	s16b tile_hgt;

	s16b size_wid;
	s16b size_hgt;

	s16b size_ow1;
	s16b size_oh1;
	s16b size_ow2;
	s16b size_oh2;
};




#ifdef MAC_MPW

/*
 * MPW 68K compiler cannot process ToME's variable.c correctly...
 * but support for it is here, for your reference. I have tried
 * this with SC and MrC to compile Vanilla successfully.
 */
QDGlobals qd;

/*
 * File type assigner - declare them in externs.h as well.
 *
 * You still have to call
 *	fsetfileinfo(buf, _fcreator, _ftype);
 * in fd_make() and my_fopen() to assign creator and type
 * to a file.
 */
u32b _ftype;
u32b _fcreator;

/*
 * Since MPW's C library doesn't have stat or fstat, you have to
 * disable CHECK_MODIFICATION_TIME in config.h
 *
 * Another source code change required for MPW compilation is
 * to #define MACINTOSH and #undef __STDC__
 * This can be done conveniently in h-system.h
 *
 * You may have to cast some pointers to non-offending types e.g. (void *).
 * This typically occurs when passing a const pointer to a library
 * function whose prototype doesn't have const in corresponding
 * parameter.
 */

#endif /* MAC_MPW */


/*
 * Forward declare -- see below
 */
static bool CheckEvents(bool wait);


/*
 * Hack -- location of the main directory
 */
static short app_vol;
static long app_dir;


/*
 * Delay handling of double-clicked savefiles
 */
Boolean open_when_ready = FALSE;

/*
 * Delay handling of pre-emptive "quit" event
 */
Boolean quit_when_ready = FALSE;


/*
 * Hack -- game in progress
 */
static int game_in_progress = 0;


/*
 * Only do "SetPort()" when needed
 */
static WindowPtr active = NULL;


/*
 * Maximum number of terms
 */
#define MAX_TERM_DATA 8


/*
 * An array of term_data's
 */
static term_data data[MAX_TERM_DATA];



/*
 * Note when "open"/"new" become valid
 */
static bool initialized = FALSE;



#ifdef ALLOW_NO_SAVE_QUITS

/*
 * CodeWarrior uses Universal Procedure Pointers
 */
static ModalFilterUPP ynfilterUPP;

#endif /* ALLOW_NO_SAVE_QUITS */



#ifdef USE_SFL_CODE

/*
 * Apple Event Hooks
 */
AEEventHandlerUPP AEH_Start_UPP;
AEEventHandlerUPP AEH_Quit_UPP;
AEEventHandlerUPP AEH_Print_UPP;
AEEventHandlerUPP AEH_Open_UPP;

#endif



/*
 * Convert a C string to a pascal string in place
 *
 * This function may be defined elsewhere, but since it is so
 * small, it is not worth finding the proper function name for
 * all the different platforms.
 */
static void ctopstr(StringPtr src)
{
	int i;
	byte len;

	/* Hack -- pointer */
	char *s = (char*)(src);

	len = strlen(s);

	/* Hack -- convert the string */
	for (i = len; i > 1; i--) s[i] = s[i - 1];

	/* Hack -- terminate the string */
	s[0] = len;
}


/*
 * Convert refnum+vrefnum+fname into a full file name
 * Store this filename in 'buf' (make sure it is long enough)
 * Note that 'fname' looks to be a "pascal" string
 */
static void refnum_to_name(char *buf, long refnum, short vrefnum, char *fname)
{
	DirInfo pb;
	Str255 name;
	int err;
	int i, j;

	char res[1000];

	i = 999;

	res[i] = 0;
	i--;
	for (j = 1; j <= fname[0]; j++)
	{
		res[i - fname[0] + j] = fname[j];
	}
	i -= fname[0];

	pb.ioCompletion = NULL;
	pb.ioNamePtr = name;
	pb.ioVRefNum = vrefnum;
	pb.ioDrParID = refnum;
	pb.ioFDirIndex = -1;

	while (1)
	{
		pb.ioDrDirID = pb.ioDrParID;
		err = PBGetCatInfoSync((CInfoPBPtr) & pb);
		res[i] = ':';
		i--;
		for (j = 1; j <= name[0]; j++)
		{
			res[i - name[0] + j] = name[j];
		}
		i -= name[0];

		if (pb.ioDrDirID == fsRtDirID) break;
	}

	/* Extract the result */
	for (j = 0, i++; res[i]; j++, i++) buf[j] = res[i];
	buf[j] = 0;
}


#if 0

/*
 * XXX XXX XXX Allow the system to ask us for a filename
 */
static bool askfor_file(char *buf, int len)
{
	SFReply reply;
	Str255 dflt;
	Point topleft;
	short vrefnum;
	long drefnum, junk;

	/* Default file name */
	strnfmt((char*)dflt + 1, 255, "%s's description", buf);
	dflt[0] = strlen((char*)dflt + 1);

	/* Ask for a file name */
	topleft.h = (qd.screenBits.bounds.left + qd.screenBits.bounds.right) / 2 - 344 / 2;
	topleft.v = (2 * qd.screenBits.bounds.top + qd.screenBits.bounds.bottom) / 3 - 188 / 2;
	SFPutFile(topleft, "\pSelect a filename:", dflt, NULL, &reply);
	/* StandardPutFile("\pSelect a filename:", dflt, &reply); */

	/* Process */
	if (reply.good)
	{
		int fc;

		/* Get info */
		GetWDInfo(reply.vRefNum, &vrefnum, &drefnum, &junk);

		/* Extract the name */
		refnum_to_name(buf, drefnum, vrefnum, (char*)reply.fName);

		/* Success */
		return (TRUE);
	}

	/* Failure */
	return (FALSE);
}

#endif



/*
 * Center a rectangle inside another rectangle
 */
static void center_rect(Rect *r, Rect *s)
{
	int centerx = (s->left + s->right) / 2;
	int centery = (2 * s->top + s->bottom) / 3;
	int dx = centerx - (r->right - r->left) / 2 - r->left;
	int dy = centery - (r->bottom - r->top) / 2 - r->top;
	r->left += dx;
	r->right += dx;
	r->top += dy;
	r->bottom += dy;
}


/*
 * Convert a pascal string in place
 *
 * This function may be defined elsewhere, but since it is so
 * small, it is not worth finding the proper function name for
 * all the different platforms.
 */
static void ptocstr(StringPtr src)
{
	int i;

	/* Hack -- pointer */
	char *s = (char*)(src);

	/* Hack -- convert the string */
	for (i = s[0]; i; i--, s++) s[0] = s[1];

	/* Hack -- terminate the string */
	s[0] = '\0';
}


#if defined(USE_SFL_CODE)


/*
 * The following three routines (pstrcat, pstrinsert, and PathNameFromDirID)
 * were taken from the Think Reference section called "Getting a Full Pathname"
 * (under the File Manager section).  We need PathNameFromDirID to get the
 * full pathname of the opened savefile, making no assumptions about where it
 * is.
 *
 * I had to hack PathNameFromDirID a little for MetroWerks, but it's awfully
 * nice.
 */
static void pstrcat(StringPtr dst, StringPtr src)
{
	/* copy string in */
	BlockMove(src + 1, dst + *dst + 1, *src);

	/* adjust length byte */
	*dst += *src;
}

/*
 * pstrinsert - insert string 'src' at beginning of string 'dst'
 */
static void pstrinsert(StringPtr dst, StringPtr src)
{
	/* make room for new string */
	BlockMove(dst + 1, dst + *src + 1, *dst);

	/* copy new string in */
	BlockMove(src + 1, dst + 1, *src);

	/* adjust length byte */
	*dst += *src;
}

static void PathNameFromDirID(long dirID, short vRefNum, StringPtr fullPathName)
{
	CInfoPBRec block;
	Str255 directoryName;
	OSErr err;

	fullPathName[0] = '\0';

	block.dirInfo.ioDrParID = dirID;
	block.dirInfo.ioNamePtr = directoryName;

	while (1)
	{
		block.dirInfo.ioVRefNum = vRefNum;
		block.dirInfo.ioFDirIndex = -1;
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
		err = PBGetCatInfoSync(&block);
		pstrcat(directoryName, (StringPtr)"\p:");
		pstrinsert(fullPathName, directoryName);
		if (block.dirInfo.ioDrDirID == 2) break;
	}
}

#endif



/*
 * Activate a given window, if necessary
 */
static void activate(WindowPtr w)
{
	/* Activate */
	if (active != w)
	{
		/* Activate */
		if (w) SetPort(w);

		/* Remember */
		active = w;
	}
}


/*
 * Display a warning message
 */
static void mac_warning(cptr warning)
{
	Str255 text;
	int len, i;

	/* Limit of 250 chars */
	len = strlen(warning);
	if (len > 250) len = 250;

	/* Make a "Pascal" string */
	text[0] = len;
	for (i = 0; i < len; i++) text[i + 1] = warning[i];

	/* Prepare the dialog box values */
	ParamText(text, "\p", "\p", "\p");

	/* Display the Alert, wait for Okay */
	Alert(129, 0L);
}



/*** Some generic functions ***/


#ifdef ANGBAND_LITE_MAC

/*
 * Hack -- activate a color (0 to 255)
 */
#define term_data_color(TD,A) /* Nothing */

#else /* ANGBAND_LITE_MAC */

/*
* Hack -- activate a color (0 to 255)
*/
static void term_data_color(term_data *td, int a)
{
	/* Activate the color */
	if (td->last != a)
	{
		/* Activate the color */
		RGBForeColor(&color_info[a]);

		/* Memorize color */
		td->last = a;
	}
}

#endif /* ANGBAND_LITE_MAC */


/*
 * Hack -- Apply and Verify the "font" info
 *
 * This should usually be followed by "term_data_check_size()"
 *
 * XXX XXX To force (re)initialisation of td->tile_wid and td->tile_hgt
 * you have to reset them to zero before this function is called.
 * XXX XXX This is automatic when the program starts because the term_data
 * array is WIPE'd by term_data_hack, but isn't in the other cases, i.e.
 * font, font style and size changes.
 */
static void term_data_check_font(term_data *td)
{
	int i;

	FontInfo info;

	WindowPtr old = active;


	/* Activate */
	activate(td->w);

	/* Instantiate font */
	TextFont(td->font_id);
	TextSize(td->font_size);
	TextFace(td->font_face);

	/* Extract the font info */
	GetFontInfo(&info);

	/* Assume monospaced */
	td->font_mono = TRUE;

	/* Extract the font sizing values XXX XXX XXX */
	td->font_wid = CharWidth('@');  /* info.widMax; */
	td->font_hgt = info.ascent + info.descent;
	td->font_o_x = 0;
	td->font_o_y = info.ascent;

	/* Check important characters */
	for (i = 33; i < 127; i++)
	{
		/* Hack -- notice non-mono-space */
		if (td->font_wid != CharWidth(i)) td->font_mono = FALSE;

		/* Hack -- collect largest width */
		if (td->font_wid < CharWidth(i)) td->font_wid = CharWidth(i);
	}

	/* Set default offsets */
	td->tile_o_x = td->font_o_x;
	td->tile_o_y = td->font_o_y;

	/* Set default tile size */
	if (td->tile_wid == 0) td->tile_wid = td->font_wid;
	if (td->tile_hgt == 0) td->tile_hgt = td->font_hgt;

	/* Re-activate the old window */
	activate(old);
}


/*
 * Hack -- Apply and Verify the "size" info
 */
static void term_data_check_size(term_data *td)
{
	/* Minimal window size for the Angband window */
	if (td == &data[0])
	{
#ifdef ALLOW_BIG_SCREEN

		/* Enforce minimal size */
		if (td->cols < 80) td->cols = 80;
		if (td->rows < 24) td->rows = 24;

#else

		/* Enforce the traditional size */
		if (td->cols != 80) td->cols = 80;
		if (td->rows != 24) td->rows = 24;

#endif /* ALLOW_BIG_SCREEN */
	}

	/* Allow small windows for the rest */
	else
	{
		if (td->cols < 1) td->cols = 1;
		if (td->rows < 1) td->rows = 1;
	}

	/* Enforce maximal sizes */
	if (td->cols > 255) td->cols = 255;
	if (td->rows > 255) td->rows = 255;

	/* Minimal tile size */
	if (td->tile_wid < td->font_wid) td->tile_wid = td->font_wid;
	if (td->tile_hgt < td->font_hgt) td->tile_hgt = td->font_hgt;

	/* Default tile offsets */
	td->tile_o_x = (td->tile_wid - td->font_wid) / 2;
	td->tile_o_y = (td->tile_hgt - td->font_hgt) / 2;

	/* Minimal tile offsets */
	if (td->tile_o_x < 0) td->tile_o_x = 0;
	if (td->tile_o_y < 0) td->tile_o_y = 0;

	/* Apply font offsets */
	td->tile_o_x += td->font_o_x;
	td->tile_o_y += td->font_o_y;

	/* Calculate full window size */
	td->size_wid = td->cols * td->tile_wid + td->size_ow1 + td->size_ow2;
	td->size_hgt = td->rows * td->tile_hgt + td->size_oh1 + td->size_oh2;

	/* Verify the top */
	if (td->r.top > qd.screenBits.bounds.bottom - td->size_hgt)
	{
		td->r.top = qd.screenBits.bounds.bottom - td->size_hgt;
	}

	/* Verify the top */
	if (td->r.top < qd.screenBits.bounds.top + 30)
	{
		td->r.top = qd.screenBits.bounds.top + 30;
	}

	/* Verify the left */
	if (td->r.left > qd.screenBits.bounds.right - td->size_wid)
	{
		td->r.left = qd.screenBits.bounds.right - td->size_wid;
	}

	/* Verify the left */
	if (td->r.left < qd.screenBits.bounds.left)
	{
		td->r.left = qd.screenBits.bounds.left;
	}

	/* Calculate bottom right corner */
	td->r.right = td->r.left + td->size_wid;
	td->r.bottom = td->r.top + td->size_hgt;

	/* Assume no graphics */
	td->t->higher_pict = FALSE;
	td->t->always_pict = FALSE;

#ifdef ANGBAND_LITE_MAC

	/* No graphics */

#else /* ANGBAND_LITE_MAC */

	/* Handle graphics */
	if (use_graphics)
	{
		/* Use higher_pict whenever possible */
		if (td->font_mono) td->t->higher_pict = TRUE;

		/* Use always_pict only when necessary */
		else td->t->always_pict = TRUE;
	}

#endif /* ANGBAND_LITE_MAC */

	/* Fake mono-space */
	if (!td->font_mono ||
	                (td->font_wid != td->tile_wid) ||
	                (td->font_hgt != td->tile_hgt))
	{
		/* Handle fake monospace -- this is SLOW */
		if (td->t->higher_pict) td->t->higher_pict = FALSE;
		td->t->always_pict = TRUE;
	}
}


/*
 * Hack -- resize a term_data
 *
 * This should normally be followed by "term_data_resize()"
 */
static void term_data_resize(term_data *td)
{
	/* Actually resize the window */
	SizeWindow(td->w, td->size_wid, td->size_hgt, 0);
}



/*
 * Hack -- redraw a term_data
 *
 * Note that "Term_redraw()" calls "TERM_XTRA_CLEAR"
 */
static void term_data_redraw(term_data *td)
{
	term *old = Term;

	/* Activate the term */
	Term_activate(td->t);

	/* Redraw the contents */
	Term_redraw();

	/* Flush the output */
	Term_fresh();

	/* Restore the old term */
	Term_activate(old);

	/* No need to redraw */
	ValidRect(&td->w->portRect);
}




#ifdef ANGBAND_LITE_MAC

/* No graphics */

#else /* ANGBAND_LITE_MAC */


/*
* Graphics support
*/

/* Set by Term_xtra_mac_react */
static int pictID; 		/* PICT id of image tiles */

static int grafWidth; 	/* Width of a tile in pixels */
static int grafHeight; 	/* Height of a tile in pixels */

/* Calculated by PICT loading code */
static int pictCols; 	/* Number of columns in tiles */
static int pictRows; 	/* Number of rows in tiles */

/* Available graphics modes */
#define GRAF_MODE_NONE	0	/* plain ASCII */
#define GRAF_MODE_8X8	1	/* 8x8 tiles, no transparency effect */
#define GRAF_MODE_16X16	2	/* 16x16 tiles, transparency effect */
/* This doesn't work... */
#define GRAF_MODE_32X32	3	/* 32x32 tiles, transparency effect */

static int graf_mode = GRAF_MODE_NONE; 		/* current graphics mode */
static int graf_mode_req = GRAF_MODE_NONE; 	/* requested graphics mode */

#define TR_NONE 0	/* No transparency effect */
#define TR_OVER 1	/* Overwriting with transparent black pixels */
static int transparency_mode = TR_NONE;


/*
* Forward Declare
*/
typedef struct FrameRec FrameRec;

/*
* Frame
*
*	- GWorld for the frame image
*	- Handle to pix map (saved for unlocking/locking)
*	- Pointer to color pix map (valid only while locked)
*/
struct FrameRec
{
	GWorldPtr framePort;
	PixMapHandle framePixHndl;
	PixMapPtr framePix;
};


/*
* The global picture data
*/
static FrameRec *frameP = NULL;


/*
* Lock a frame
*/
static void BenSWLockFrame(FrameRec *srcFrameP)
{
	PixMapHandle pixMapH;

	pixMapH = GetGWorldPixMap(srcFrameP->framePort);
	(void)LockPixels(pixMapH);
	HLockHi((Handle)pixMapH);
	srcFrameP->framePixHndl = pixMapH;
	srcFrameP->framePix = (PixMapPtr)StripAddress(*(Handle)pixMapH);
}


/*
* Unlock a frame
*/
static void BenSWUnlockFrame(FrameRec *srcFrameP)
{
	if (srcFrameP->framePort != NULL)
	{
		HUnlock((Handle)srcFrameP->framePixHndl);
		UnlockPixels(srcFrameP->framePixHndl);
	}

	srcFrameP->framePix = NULL;
}



static OSErr BenSWCreateGWorldFromPict(
        GWorldPtr *pictGWorld, PicHandle pictH)
{
	OSErr err;
	GWorldPtr saveGWorld;
	GDHandle saveGDevice;
	GWorldPtr tempGWorld;
	Rect pictRect;
	short depth;
	GDHandle theGDH;

	tempGWorld = NULL;

	/* Reset */
	*pictGWorld = NULL;

	/* Get depth */
	depth = data[0].pixelDepth;

	/* Get GDH */
	theGDH = data[0].theGDH;

	/* Obtain size rectangle */
	pictRect = (**pictH).picFrame;
	OffsetRect(&pictRect, -pictRect.left, -pictRect.top);

	/* Calculate and set numbers of rows and columns */
	pictRows = pictRect.bottom / grafHeight;
	pictCols = pictRect.right / grafWidth;

	/* Create a GWorld */
	err = NewGWorld(&tempGWorld, depth, &pictRect, nil, theGDH, noNewDevice);

	/* Error */
	if (err != noErr) return (err);

	/* Save pointer */
	*pictGWorld = tempGWorld;

	/* Save GWorld */
	GetGWorld(&saveGWorld, &saveGDevice);

	/* Activate */
	SetGWorld(tempGWorld, nil);

	/* Dump the pict into the GWorld */
	(void)LockPixels(GetGWorldPixMap(tempGWorld));
	EraseRect(&pictRect);
	DrawPicture(pictH, &pictRect);
	UnlockPixels(GetGWorldPixMap(tempGWorld));

	/* Restore GWorld */
	SetGWorld(saveGWorld, saveGDevice);

	/* Success */
	return (0);
}


/*
* Init the global "frameP"
*/
static OSErr globe_init(void)
{
	OSErr err;

	GWorldPtr tempPictGWorldP;

	PicHandle newPictH;


	/* Use window XXX XXX XXX */
	SetPort(data[0].w);


	/* Get the pict resource */
	newPictH = GetPicture(pictID);

	/* Error */
	if (newPictH == NULL) return ( -1);

	/* Create GWorld */
	err = BenSWCreateGWorldFromPict(&tempPictGWorldP, newPictH);

	/* Release resource */
	ReleaseResource((Handle)newPictH);

	/* Error */
	if (err != noErr) return (err);

	/* Create the frame */
	frameP = (FrameRec*)NewPtrClear((Size)sizeof(FrameRec));

	/* Error */
	if (frameP == NULL) return ( -1);

	/* Save GWorld */
	frameP->framePort = tempPictGWorldP;

	/* Lock it */
	BenSWLockFrame(frameP);

	/* Success */
	return (noErr);
}


/*
* Nuke the global "frameP"
*/
static errr globe_nuke(void)
{
	/* Dispose */
	if (frameP)
	{
		/* Unlock */
		BenSWUnlockFrame(frameP);

		/* Dispose of the GWorld */
		DisposeGWorld(frameP->framePort);

		/* Dispose of the memory */
		DisposePtr((Ptr)frameP);

		/* Forget */
		frameP = NULL;
	}

	/* Flush events */
	FlushEvents(everyEvent, 0);

	/* Success */
	return (0);
}


#endif /* ANGBAND_LITE_MAC */




#ifdef USE_ASYNC_SOUND

/*
 * Asynchronous sound player - completely revised (beta)
 */

/*
 * Number of channels in the channel pool
 */
#define MAX_CHANNELS 4

/*
 * A pool of sound channels
 */
static SndChannelPtr channels[MAX_CHANNELS];

/*
 * Status of the channel pool
 */
static Boolean channel_initialised = FALSE;

/*
 * Data handles containing sound samples
 */
static SndListHandle samples[SOUND_MAX];

/*
 * Reference counts of sound samples
 */
static SInt16 sample_refs[SOUND_MAX];

#define SOUND_VOLUME_MIN	0	/* Default minimum sound volume */
#define SOUND_VOLUME_MAX	255	/* Default maximum sound volume */
#define VOLUME_MIN			0	/* Minimum sound volume in % */
#define VOLUME_MAX			100	/* Maximum sound volume in % */
#define VOLUME_INC			5	/* Increment sound volume in % */

/* I'm just too lazy to write a panel for this XXX XXX */
static int sound_volume = SOUND_VOLUME_MAX;

/*
 * Return a handle of 'snd ' resource given Angband sound event number,
 * or NULL if it isn't found.
 *
 * Globals referenced: angband_sound_name[] (variable.c)
 */
static SndListHandle find_sound(int num)
{
	Str255 sound;

	/* Get the proper sound name */
	strnfmt((char*)sound + 1, 255, "%.16s.wav", angband_sound_name[num]);
	sound[0] = strlen((char*)sound + 1);

	/* Obtain resource XXX XXX XXX */
	return ((SndListHandle)GetNamedResource('snd ', sound));
}


/*
 * Clean up sound support - to be called when the game exits.
 *
 * Globals referenced: channels[], samples[], sample_refs[].
 */
static void cleanup_sound(void)
{
	int i;

	/* No need to clean it up */
	if (!channel_initialised) return;

	/* Dispose channels */
	for (i = 0; i < MAX_CHANNELS; i++)
	{
		/* Drain sound commands and free the channel */
		SndDisposeChannel(channels[i], TRUE);
	}

	/* Free sound data */
	for (i = 1; i < SOUND_MAX; i++)
	{
		/* Still locked */
		if ((sample_refs[i] > 0) && (samples[i] != NULL))
		{
			/* Unlock it */
			HUnlock((Handle)samples[i]);
		}

		/* Release it */
		if (samples[i]) ReleaseResource((Handle)samples[i]);
	}
}


/*
 * Play sound effects asynchronously -- pelpel
 *
 * I don't believe those who first started using the previous implementations
 * imagined this is *much* more complicated as it may seem.  Anyway, 
 * introduced round-robin scheduling of channels and made it much more
 * paranoid about HLock/HUnlock.
 *
 * XXX XXX de-refcounting, HUnlock and ReleaseResource should be done
 * using channel's callback procedures, which set global flags, and
 * a procedure hooked into CheckEvents does housekeeping.  On the other
 * hand, this lazy reclaiming strategy keeps things simple (no interrupt
 * time code) and provides a sort of cache for sound data.
 *
 * Globals referenced: channel_initialised, channels[], samples[],
 *   sample_refs[].
 * Globals updated: ditto.
 */
static void play_sound(int num, int vol)
{
	OSErr err;
	int i;
	int prev_num;
	SndListHandle h;
	SndChannelPtr chan;
	SCStatus status;

	static int next_chan;
	static SInt16 channel_occupants[MAX_CHANNELS];
	static SndCommand volume_cmd, quiet_cmd;


	/* Initialise sound channels */
	if (!channel_initialised)
	{
		for (i = 0; i < MAX_CHANNELS; i++)
		{
			/* Paranoia - Clear occupant table */
			/* channel_occupants[i] = 0; */

			/* Create sound channel for all sounds to play from */
			err = SndNewChannel(&channels[i], sampledSynth, initMono, 0L);

			/* Error */
			if (err != noErr)
			{
				/* Free channels */
				while (--i >= 0)
				{
					SndDisposeChannel(channels[i], TRUE);
				}

				/* Notify error */
				plog("Cannot initialise sound channels!");

				/* Cancel request */
				use_sound = arg_sound = FALSE;

				/* Failure */
				return;
			}
		}

		/* First channel to use */
		next_chan = 0;

		/* Prepare volume command */
		volume_cmd.cmd = volumeCmd;
		volume_cmd.param1 = 0;
		volume_cmd.param2 = 0;

		/* Prepare quiet command */
		quiet_cmd.cmd = quietCmd;
		quiet_cmd.param1 = 0;
		quiet_cmd.param2 = 0;

		/* Initialisation complete */
		channel_initialised = TRUE;
	}

	/* Paranoia */
	if ((num <= 0) || (num >= SOUND_MAX)) return;

	/* Prepare volume command */
	volume_cmd.param2 = (SInt16)((vol << 4) | vol);

	/* Channel to use (round robin) */
	chan = channels[next_chan];

	/* See if the resource is already in use */
	if (sample_refs[num] > 0)
	{
		/* Resource in use */
		h = samples[num];

		/* Increase the refcount */
		sample_refs[num]++;
	}

	/* Sound is not currently in use */
	else
	{
		/* Get handle for the sound */
		h = find_sound(num);

		/* Sample not available */
		if (h == NULL) return;

		/* Load resource */
		LoadResource((Handle)h);

		/* Lock the handle */
		HLock((Handle)h);

		/* Remember it */
		samples[num] = h;

		/* Initialise refcount */
		sample_refs[num] = 1;
	}

	/* Poll the channel */
	err = SndChannelStatus(chan, sizeof(SCStatus), &status);

	/* It isn't available */
	if ((err != noErr) || status.scChannelBusy)
	{
		/* Shut it down */
		SndDoImmediate(chan, &quiet_cmd);
	}

	/* Previously played sound on this channel */
	prev_num = channel_occupants[next_chan];

	/* Process previously played sound */
	if (prev_num != 0)
	{
		/* Decrease refcount */
		sample_refs[prev_num]--;

		/* We can free it now */
		if (sample_refs[prev_num] <= 0)
		{
			/* Unlock */
			HUnlock((Handle)samples[prev_num]);

			/* Release */
			ReleaseResource((Handle)samples[prev_num]);

			/* Forget handle */
			samples[prev_num] = NULL;

			/* Paranoia */
			sample_refs[prev_num] = 0;
		}
	}

	/* Remember this sound as the current occupant of the channel */
	channel_occupants[next_chan] = num;

	/* Set up volume for channel */
	SndDoImmediate(chan, &volume_cmd);

	/* Play new sound asynchronously */
	SndPlay(chan, h, TRUE);

	/* Schedule next channel (round robin) */
	next_chan++;
	if (next_chan >= MAX_CHANNELS) next_chan = 0;
}

#endif /* USE_ASYNC_SOUND */


/*** Support for the "z-term.c" package ***/


/*
 * Initialize a new Term
 *
 * Note also the "window type" called "noGrowDocProc", which might be more
 * appropriate for the main "screen" window.
 *
 * Note the use of "srcCopy" mode for optimized screen writes.
 */
static void Term_init_mac(term *t)
{
	term_data *td = (term_data*)(t->data);

	static RGBColor black = {0x0000, 0x0000, 0x0000};
	static RGBColor white = {0xFFFF, 0xFFFF, 0xFFFF};

#ifdef ANGBAND_LITE_MAC

	/* Make the window */
	td->w = NewWindow(0, &td->r, td->title, 0, noGrowDocProc, (WindowPtr) - 1, 1, 0L);

#else /* ANGBAND_LITE_MAC */

	/* Make the window */
	td->w = NewCWindow(0, &td->r, td->title, 0, documentProc, (WindowPtr) - 1, 1, 0L);

#endif /* ANGBAND_LITE_MAC */

	/* Activate the window */
	activate(td->w);

	/* Erase behind words */
	TextMode(srcCopy);

	/* Apply and Verify */
	term_data_check_font(td);
	term_data_check_size(td);

	/* Resize the window */
	term_data_resize(td);

#ifdef ANGBAND_LITE_MAC

	/* Prepare the colors (base colors) */
	BackColor(blackColor);
	ForeColor(whiteColor);

#else /* ANGBAND_LITE_MAC */

	/* Prepare the colors (real colors) */
	RGBBackColor(&black);
	RGBForeColor(&white);

	/* Block */
	{
		Rect tempRect;
		Rect globalRect;
		GDHandle mainGDH;
		GDHandle currentGDH;
		GWorldPtr windowGWorld;
		PixMapHandle basePixMap;

		/* Obtain the rect */
		tempRect = td->w->portRect;

		/* Obtain the global rect */
		globalRect = tempRect;
		LocalToGlobal((Point*)&globalRect.top);
		LocalToGlobal((Point*)&globalRect.bottom);

		/* Obtain the proper GDH */
		mainGDH = GetMaxDevice(&globalRect);

		/* Extract GWorld and GDH */
		GetGWorld(&windowGWorld, &currentGDH);

		/* Obtain base pixmap */
		basePixMap = (**mainGDH).gdPMap;

		/* Save pixel depth */
		td->pixelDepth = (**basePixMap).pixelSize;

		/* Save Window GWorld */
		td->theGWorld = windowGWorld;

		/* Save Window GDH */
		td->theGDH = currentGDH;

		/* Save main GDH */
		td->mainSWGDH = mainGDH;
	}

#endif /* ANGBAND_LITE_MAC */

	/* Clip to the window */
	ClipRect(&td->w->portRect);

	/* Erase the window */
	EraseRect(&td->w->portRect);

	/* Invalidate the window */
	InvalRect(&td->w->portRect);

	/* Display the window if needed */
	if (td->mapped) ShowWindow(td->w);

	/* Hack -- set "mapped" flag */
	t->mapped_flag = td->mapped;

	/* Forget color */
	td->last = -1;
}



/*
 * Nuke an old Term
 */
static void Term_nuke_mac(term *t)
{

#pragma unused (t)

	/* XXX */
}



/*
 * Unused
 */
static errr Term_user_mac(int n)
{

#pragma unused (n)

	/* Success */
	return (0);
}



/*
 * React to changes
 */
static errr Term_xtra_mac_react(void)
{
	term_data *td = (term_data*)(Term->data);

	int i;


#ifdef ANGBAND_LITE_MAC

	/* Nothing */

#else /* ANGBAND_LITE_MAC */

	/* Reset color */
	td->last = -1;

	/* Update colors */
	for (i = 0; i < 256; i++)
	{
		u16b rv, gv, bv;

		/* Extract the R,G,B data */
		rv = angband_color_table[i][1];
		gv = angband_color_table[i][2];
		bv = angband_color_table[i][3];

		/* Save the actual color */
		color_info[i].red = (rv | (rv << 8));
		color_info[i].green = (gv | (gv << 8));
		color_info[i].blue = (bv | (bv << 8));
	}

	/* Handle sound */
	if (use_sound != arg_sound)
	{
		/* Apply request */
		use_sound = arg_sound;
	}

	/* Handle graphics */
	if (graf_mode_req != graf_mode)
	{
		/* dispose old GWorld's if present */
		globe_nuke();

		/* Setup parameters according to request */
		switch (graf_mode_req)
		{
			/* ASCII - no graphics whatsoever */
		case GRAF_MODE_NONE:
			{
#ifndef ZANGBAND
				use_graphics = arg_graphics = FALSE;
#else
use_graphics = arg_graphics = GRAPHICS_NONE;
#endif /* !ZANGBAND */
				transparency_mode = TR_NONE;
				break;
			}

			/*
			* 8x8 tiles (PICT id 1001)
			* no transparency effect
			* "old" graphics definitions
			*/
		case GRAF_MODE_8X8:
			{
#ifndef ZANGBAND
				use_graphics = arg_graphics = TRUE;
				ANGBAND_GRAF = "old";
#else
	use_graphics = arg_graphics = GRAPHICS_ORIGINAL;
#endif /* !ZANGBAND */
				pictID = 1001;
				transparency_mode = TR_NONE;
				grafWidth = grafHeight = 8;
				break;
			}

			/*
			* 16x16 tiles (images: PICT id 1002)
			* with transparency effect
			* "new" graphics definitions
			*/
		case GRAF_MODE_16X16:
			{
#ifndef ZANGBAND
				use_graphics = arg_graphics = TRUE;
				ANGBAND_GRAF = "new";
#else
	use_graphics = arg_graphics = GRAPHICS_ADAM_BOLT;
#endif /* !ZANGBAND */
				pictID = 1002;
				transparency_mode = TR_OVER;
				grafWidth = grafHeight = 16;
				break;
			}

#if 0 /* Good Lord! It doesn't work... Is it too large? */

			/*
			* 32x32 tiles (images: PICT id 1004)
			* no transparency effect
			* "david" graphics definitions
			*/
		case GRAF_MODE_32X32:
			{
				use_graphics = arg_graphics = TRUE;
				ANGBAND_GRAF = "david";
				pictID = 1004;
				transparency_mode = TR_OVER;
				grafWidth = grafHeight = 32;
				break;
			}

#endif /* sigh */
		}

		/* load tiles and setup GWorlds if tiles are requested */
		if ((graf_mode_req != GRAF_MODE_NONE) && (globe_init() != 0))
		{
			/* Oops */
			plog("Cannot initialize graphics!");

			/* reject request */
			graf_mode_req = GRAF_MODE_NONE;

			/* reset graphics flags */
			use_graphics = arg_graphics = FALSE;

			/* reset transparency mode */
			transparency_mode = TR_NONE;
		}

		/* update current graphics mode */
		graf_mode = graf_mode_req;

		/* Apply and Verify */
		term_data_check_size(td);

		/* Resize the window */
		term_data_resize(td);

		/* Reset visuals */
#ifndef ANG281_RESET_VISUALS
		reset_visuals(TRUE);
#else
reset_visuals();
#endif /* !ANG281_RESET_VISUALS */
	}

#endif /* ANGBAND_LITE_MAC */

	/* Success */
	return (0);
}


/*
 * Do a "special thing"
 */
static errr Term_xtra_mac(int n, int v)
{
	term_data *td = (term_data*)(Term->data);

	Rect r;

	/* Analyze */
	switch (n)
	{
		/* Make a noise */
	case TERM_XTRA_NOISE:
		{
			/* Make a noise */
			SysBeep(1);

			/* Success */
			return (0);
		}

#ifdef ANGBAND_LITE_MAC

		/* Nothing */

#else /* ANGBAND_LITE_MAC */

		/* Make a sound */
	case TERM_XTRA_SOUND:
		{
#ifndef USE_ASYNC_SOUND

			/*
			* This may not be your choice, but much safer and much less
			* resource hungry.  Existing implementations can quite easily
			* crash, by starting asynchronous playing and immediately
			* unlocking and releasing the sound data just started playing...
			* -- pelpel
			*/
			Handle handle;
			Str255 sound;

			/* Get the proper sound name */
			strnfmt((char*)sound + 1, 255, "%.16s.wav", angband_sound_name[v]);
			sound[0] = strlen((char*)sound + 1);

			/* Obtain resource XXX XXX XXX */
			handle = GetNamedResource('snd ', sound);

			/* Oops -- it is a failure, but we return 0 anyway */
			if (handle == NULL) return (0);

			/* Load and Lock */
			LoadResource(handle);
			HLock(handle);

			/* Play sound (wait for completion) */
			SndPlay(nil, (SndListHandle)handle, false);

			/* Unlock and release */
			HUnlock(handle);
			ReleaseResource(handle);

#else /* !USE_ASYNC_SOUND */

		/* If you really want async player, please try this one */
		play_sound(v, sound_volume);

#endif /* !USE_ASYNC_SOUND */

			/* Success */
			return (0);
		}

#endif /* ANGBAND_LITE_MAC */

		/* Process random events */
	case TERM_XTRA_BORED:
		{
			/* Process an event */
			(void)CheckEvents(FALSE);

			/* Success */
			return (0);
		}

		/* Process pending events */
	case TERM_XTRA_EVENT:
		{
			/* Process an event */
			(void)CheckEvents(v);

			/* Success */
			return (0);
		}

		/* Flush all pending events (if any) */
	case TERM_XTRA_FLUSH:
		{
			/* Hack -- flush all events */
			while (CheckEvents(TRUE)) /* loop */;

			/* Success */
			return (0);
		}

		/* Hack -- Change the "soft level" */
	case TERM_XTRA_LEVEL:
		{
			/* Activate if requested */
			if (v) activate(td->w);

			/* Success */
			return (0);
		}

		/* Clear the screen */
	case TERM_XTRA_CLEAR:
		{
			/* No clipping XXX XXX XXX */
			ClipRect(&td->w->portRect);

			/* Erase the window */
			EraseRect(&td->w->portRect);

			/* Set the color */
			term_data_color(td, TERM_WHITE);

			/* Frame the window in white */
			MoveTo(0, 0);
			LineTo(0, td->size_hgt - 1);
			LineTo(td->size_wid - 1, td->size_hgt - 1);
			LineTo(td->size_wid - 1, 0);

			/* Clip to the new size */
			r.left = td->w->portRect.left + td->size_ow1;
			r.top = td->w->portRect.top + td->size_oh1;
			r.right = td->w->portRect.right - td->size_ow2;
			r.bottom = td->w->portRect.bottom - td->size_oh2;
			ClipRect(&r);

			/* Success */
			return (0);
		}

		/* React to changes */
	case TERM_XTRA_REACT:
		{
			/* React to changes */
			return (Term_xtra_mac_react());
		}

		/* Delay (milliseconds) */
	case TERM_XTRA_DELAY:
		{
			/* If needed */
			if (v > 0)
			{
				long m = TickCount() + (v * 60L) / 1000;

				/* Wait for it */
				while (TickCount() < m) /* loop */;
			}

			/* Success */
			return (0);
		}

		/* Rename main window */
	case TERM_XTRA_RENAME_MAIN_WIN:
		{
			char *s = strdup(angband_term_name[0]);

			ctopstr((StringPtr)s);
			SetWTitle(data[0].w, (StringPtr)s);

			free(s);
			return (0);
		}
	}

	/* Oops */
	return (1);
}



/*
 * Low level graphics (Assumes valid input).
 * Draw a "cursor" at (x,y), using a "yellow box".
 * We are allowed to use "Term_what()" to determine
 * the current screen contents (for inverting, etc).
 */
static errr Term_curs_mac(int x, int y)
{
	Rect r;

	term_data *td = (term_data*)(Term->data);

	/* Set the color */
	term_data_color(td, TERM_YELLOW);

	/* Frame the grid */
	r.left = x * td->tile_wid + td->size_ow1;
	r.right = r.left + td->tile_wid;
	r.top = y * td->tile_hgt + td->size_oh1;
	r.bottom = r.top + td->tile_hgt;

#ifdef USE_DOUBLE_TILES

	/* Mogami's double width tile patch */
	if (use_bigtile &&
	                (x + 1 < Term->wid) &&
	                (Term->old->a[y][x + 1] == 255))
	{
		r.right += td->tile_wid;
	}

#endif /* USE_DOUBLE_TILES */

	FrameRect(&r);

	/* Success */
	return (0);
}


/*
 * Low level graphics (Assumes valid input)
 *
 * Erase "n" characters starting at (x,y)
 */
static errr Term_wipe_mac(int x, int y, int n)
{
	Rect r;

	term_data *td = (term_data*)(Term->data);

	/* Erase the block of characters */
	r.left = x * td->tile_wid + td->size_ow1;
	r.right = r.left + n * td->tile_wid;
	r.top = y * td->tile_hgt + td->size_oh1;
	r.bottom = r.top + td->tile_hgt;
	EraseRect(&r);

	/* Success */
	return (0);
}


/*
 * Low level graphics.  Assumes valid input.
 *
 * Draw several ("n") chars, with an attr, at a given location.
 */
static errr Term_text_mac(int x, int y, int n, byte a, const char *cp)
{
	int xp, yp;

	term_data *td = (term_data*)(Term->data);

	/* Set the color */
	term_data_color(td, a);

	/* Starting pixel */
	xp = x * td->tile_wid + td->tile_o_x + td->size_ow1;
	yp = y * td->tile_hgt + td->tile_o_y + td->size_oh1;

	/* Move to the correct location */
	MoveTo(xp, yp);

	/* Draw the character */
	if (n == 1) DrawChar(*cp);

	/* Draw the string */
	else DrawText(cp, 0, n);

	/* Success */
	return (0);
}


/*
 * Low level graphics (Assumes valid input)
 *
 * Erase "n" characters starting at (x,y)
 */
#ifdef USE_TRANSPARENCY
# ifdef USE_EGO_GRAPHICS
static errr Term_pict_mac(int x, int y, int n, const byte *ap, const char *cp,
                          const byte *tap, const char *tcp,
                          const byte *eap, const char *ecp)
# else /* USE_EGO_GRAPHICS */
static errr Term_pict_mac(int x, int y, int n, const byte *ap, const char *cp,
                          const byte *tap, const char *tcp)
# endif  /* USE_EGO_GRAPHICS */
#else /* USE_TRANSPARENCY */
static errr Term_pict_mac(int x, int y, int n, const byte *ap, const char *cp)
#endif /* USE_TRANSPARENCY */
{
	int i;
	Rect dst_r;
	static RGBColor black = {0x0000, 0x0000, 0x0000};
	static RGBColor white = {0xFFFF, 0xFFFF, 0xFFFF};
	term_data *td = (term_data*)(Term->data);

	/* Destination rectangle */
	dst_r.left = x * td->tile_wid + td->size_ow1;
#ifndef USE_DOUBLE_TILES
	dst_r.right = dst_r.left + td->tile_wid;
#endif /* !USE_DOUBLE_TILES */
	dst_r.top = y * td->tile_hgt + td->size_oh1;
	dst_r.bottom = dst_r.top + td->tile_hgt;

	/* Scan the input */
	for (i = 0; i < n; i++)
	{
		byte a = *ap++;
		char c = *cp++;

#ifdef USE_TRANSPARENCY
		byte ta = *tap++;
		char tc = *tcp++;
# ifdef USE_EGO_GRAPHICS
		byte ea = *eap++;
		char ec = *ecp++;
		bool has_overlay = (ea && ec);
# endif  /* USE_EGO_GRAPHICS */
#endif /* USE_TRANSPARENCY */

#ifdef USE_DOUBLE_TILES

		/* Second byte of bigtile */
		if (use_bigtile && (a == 255))
		{
			/* Advance */
			dst_r.left += td->tile_wid;

			/* Ignore it */
			continue;
		}

		/* Prepare right side of rectangle */
		dst_r.right = dst_r.left + td->tile_wid;

#endif /* USE_DOUBLE_TILES */

#ifdef ANGBAND_LITE_MAC

		/* Nothing */

#else /* ANGBAND_LITE_MAC */

		/* Graphics -- if Available and Needed */
		if (use_graphics && ((byte)a & 0x80) && ((byte)c & 0x80))
		{
#ifdef USE_TRANSPARENCY
			int t_col, t_row;
			Rect terrain_r;
# ifdef USE_EGO_GRAPHICS
			int e_col, e_row;
			Rect ego_r;
# endif  /* USE_EGO_GRAPHICS */
#endif /* USE_TRANSPARENCY */

			int col, row;
			Rect src_r;

			/* Row and Col */
			row = ((byte)a & 0x7F) % pictRows;
			col = ((byte)c & 0x7F) % pictCols;

			/* Source rectangle */
			src_r.left = col * grafWidth;
			src_r.top = row * grafHeight;
			src_r.right = src_r.left + grafWidth;
			src_r.bottom = src_r.top + grafHeight;

#ifdef USE_TRANSPARENCY

			/* Terrain Row and Col */
			t_row = ((byte)ta & 0x7F) % pictRows;
			t_col = ((byte)tc & 0x7F) % pictCols;

			/* Terrain Source rectangle */
			terrain_r.left = t_col * grafWidth;
			terrain_r.top = t_row * grafHeight;
			terrain_r.right = terrain_r.left + grafWidth;
			terrain_r.bottom = terrain_r.top + grafHeight;

# ifdef USE_EGO_GRAPHICS

			/* If and only if there's overlay */
			if (has_overlay)
			{
				/* Overlay Row and Col */
				e_row = ((byte)ea & 0x7F) % pictRows;
				e_col = ((byte)ec & 0x7F) % pictCols;

				/* Overlay Source rectangle */
				ego_r.left = e_col * grafWidth;
				ego_r.top = e_row * grafHeight;
				ego_r.right = ego_r.left + grafWidth;
				ego_r.bottom = ego_r.top + grafHeight;
			}

# endif  /* USE_EGO_GRAPHICS */

#endif /* USE_TRANSPARENCY */

			/* Hardwire CopyBits */
			RGBBackColor(&white);
			RGBForeColor(&black);

#ifdef USE_DOUBLE_TILES

			/* Double width tiles */
			if (use_bigtile) dst_r.right += td->tile_wid;

#endif /* USE_DOUBLE_TILES */

#ifdef USE_TRANSPARENCY

			/* Transparency effect */
			switch (transparency_mode)
			{
				/* No transparency effect */
			case TR_NONE:
			default:
				{
					/* Draw the picture */
					CopyBits((BitMap*)frameP->framePix,
					         &(td->w->portBits),
					         &src_r, &dst_r, srcCopy, NULL);

					break;
				}

				/* Overwriting with transparent black pixels */
			case TR_OVER:
				{
					/* Draw the terrain */
					CopyBits((BitMap*)frameP->framePix,
					         &(td->w->portBits),
					         &terrain_r, &dst_r, srcCopy, NULL);

					/* Make black pixels transparent */
					RGBBackColor(&black);

					/* Draw mon/obj if there's one */
					if ((row != t_row) || (col != t_col))
						CopyBits((BitMap*)frameP->framePix,
						         &(td->w->portBits),
						         &src_r, &dst_r, transparent, NULL);

# ifdef USE_EGO_GRAPHICS

					/* Draw overlay if there's one */
					if (has_overlay)
						CopyBits((BitMap*)frameP->framePix,
						         &(td->w->portBits),
						         &ego_r, &dst_r, transparent, NULL);

# endif  /* USE_EGO_GRAPHICS */

					break;
				}
			}

#else /* USE_TRANSPARENCY */

/* Draw the picture */
		CopyBits((BitMap*)frameP->framePix,
		         &(td->w->portBits),
		         &src_r, &dst_r, srcCopy, NULL);

#endif /* USE_TRANSPARENCY */

			/* Restore colors */
			RGBBackColor(&black);
			RGBForeColor(&white);

			/* Forget color */
			td->last = -1;
		}

#endif /* ANGBAND_LITE_MAC */

		/* Normal */
		else
		{
			int xp, yp;

			/* Erase */
			EraseRect(&dst_r);

			/* Set the color */
			term_data_color(td, a);

			/* Starting pixel */
			xp = dst_r.left + td->tile_o_x;
			yp = dst_r.top + td->tile_o_y;

			/* Move to the correct location */
			MoveTo(xp, yp);

			/* Draw the character */
			DrawChar(c);
		}

		/* Advance */
		dst_r.left += td->tile_wid;
#ifndef USE_DOUBLE_TILES
		dst_r.right += td->tile_wid;
#endif /* USE_DOUBLE_TILES */
	}

	/* Success */
	return (0);
}





/*
 * Create and initialize window number "i"
 */
static void term_data_link(int i)
{
	term *old = Term;

	term_data *td = &data[i];

	/* Only once */
	if (td->t) return;

	/* Require mapped */
	if (!td->mapped) return;

	/* Allocate */
	MAKE(td->t, term);

	/* Initialize the term */
	term_init(td->t, td->cols, td->rows, td->keys);

	/* Use a "software" cursor */
	td->t->soft_cursor = TRUE;

	/* Erase with "white space" */
	td->t->attr_blank = TERM_WHITE;
	td->t->char_blank = ' ';

	/* Prepare the init/nuke hooks */
	td->t->init_hook = Term_init_mac;
	td->t->nuke_hook = Term_nuke_mac;

	/* Prepare the function hooks */
	td->t->user_hook = Term_user_mac;
	td->t->xtra_hook = Term_xtra_mac;
	td->t->wipe_hook = Term_wipe_mac;
	td->t->curs_hook = Term_curs_mac;
	td->t->text_hook = Term_text_mac;
	td->t->pict_hook = Term_pict_mac;

	/* Link the local structure */
	td->t->data = (void *)(td);

	/* Activate it */
	Term_activate(td->t);

	/* Global pointer */
	angband_term[i] = td->t;

	/* Activate old */
	Term_activate(old);
}




/*
 * Set the "current working directory" (also known as the "default"
 * volume/directory) to the location of the current application.
 *
 * Code by: Maarten Hazewinkel (mmhazewi@cs.ruu.nl)
 *
 * This function does not appear to work correctly with System 6.
 */
static void SetupAppDir(void)
{
	FCBPBRec fcbBlock;
	OSErr err = noErr;
	char errString[100];

	/* Get the location of the Angband executable */
	fcbBlock.ioCompletion = NULL;
	fcbBlock.ioNamePtr = NULL;
	fcbBlock.ioVRefNum = 0;
	fcbBlock.ioRefNum = CurResFile();
	fcbBlock.ioFCBIndx = 0;
	err = PBGetFCBInfoSync(&fcbBlock);
	if (err != noErr)
	{
		strnfmt(errString, 100, "Fatal PBGetFCBInfo Error #%d.\r Exiting.", err);
		mac_warning(errString);
		ExitToShell();
	}

	/* Extract the Vol and Dir */
	app_vol = fcbBlock.ioFCBVRefNum;
	app_dir = fcbBlock.ioFCBParID;

	/* Set the current working directory to that location */
	err = HSetVol(NULL, app_vol, app_dir);
	if (err != noErr)
	{
		strnfmt(errString, 100, "Fatal HSetVol Error #%d.\r Exiting.", err);
		mac_warning(errString);
		ExitToShell();
	}
}




/*
 * Global "preference" file pointer
 */
static FILE *fff;

/*
 * Read a "short" from the file
 */
static int getshort(void)
{
	int x = 0;
	char buf[256];
	if (0 == my_fgets(fff, buf, 256)) x = atoi(buf);
	return (x);
}

/*
 * Dump a "short" to the file
 */
static void putshort(int x)
{
	fprintf(fff, "%d\n", x);
}



/*
 * Write the "preference" data to the current "file"
 */
static void save_prefs(void)
{
	int i;

	term_data *td;


	/*** The current version ***/

	putshort(PREF_VER_MAJOR);
	putshort(PREF_VER_MINOR);
	putshort(PREF_VER_PATCH);
	putshort(PREF_VER_EXTRA);

	/* Gfx settings */
	putshort(arg_sound);
	putshort(graf_mode);
#ifdef USE_DOUBLE_TILES
	putshort(use_bigtile);
#endif /* USE_DOUBLE_TILES */

	/* Dump */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Access */
		td = &data[i];

		putshort(td->mapped);

		putshort(td->font_id);
		putshort(td->font_size);
		putshort(td->font_face);

		putshort(td->tile_wid);
		putshort(td->tile_hgt);

		putshort(td->cols);
		putshort(td->rows);

		putshort(td->r.left);
		putshort(td->r.top);
	}
}


/*
 * Load the preferences from the current "file"
 *
 * XXX XXX XXX Being able to undefine various windows is
 * slightly bizarre, and may cause problems.
 */
static void load_prefs(void)
{
	int i;

	int old_major, old_minor, old_patch, old_extra;

	term_data *td;


	/*** Version information ***/

	/* Preferences version */
	old_major = getshort();
	old_minor = getshort();
	old_patch = getshort();
	old_extra = getshort();

	/* Hack -- Verify or ignore */
	if ((old_major != PREF_VER_MAJOR) ||
	                (old_minor != PREF_VER_MINOR) ||
	                (old_patch != PREF_VER_PATCH) ||
	                (old_extra != PREF_VER_EXTRA))
	{
		/* Message */
		mac_warning("Ignoring old preferences.");

		/* Ignore */
		return;
	}

	/* Gfx mode */
	arg_sound = getshort();
	graf_mode_req = getshort();
#ifdef USE_DOUBLE_TILES
	use_bigtile = getshort();
#endif /* USE_DOUBLE_TILES */

	/* Windows */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Access */
		td = &data[i];

		td->mapped = getshort();

		td->font_id = getshort();
		td->font_size = getshort();
		td->font_face = getshort();

		td->tile_wid = getshort();
		td->tile_hgt = getshort();

		td->cols = getshort();
		td->rows = getshort();

		td->r.left = getshort();
		td->r.top = getshort();

		/* Done */
		if (feof(fff)) break;
	}
}




/*
 * Hack -- default data for a window
 */
static void term_data_hack(term_data *td)
{
	short fid;

	/* Default to Monaco font */
	GetFNum("\pmonaco", &fid);

	/* Wipe it */
	WIPE(td, term_data);

	/* No color */
	td->last = -1;

	/* Default borders */
	td->size_ow1 = 2;
	td->size_ow2 = 2;
	td->size_oh2 = 2;

	/* Start hidden */
	td->mapped = FALSE;

	/* Default font */
	td->font_id = fid;

	/* Default font size */
	td->font_size = 12;

	/* Default font face */
	td->font_face = 0;

	/* Default size */
	td->rows = 24;
	td->cols = 80;

	/* Default position */
	td->r.left = 10;
	td->r.top = 40;

	/* Minimal keys */
	td->keys = 16;
}


/*
 * Read the preference file, Create the windows.
 *
 * We attempt to use "FindFolder()" to track down the preference file,
 * but if this fails, for any reason, we will try the "SysEnvirons()"
 * method, which may work better with System 6.
 */
static void init_windows(void)
{
	int i, b = 0;

	term_data *td;

	bool oops;


	/*** Default values ***/

	/* Initialize (backwards) */
	for (i = MAX_TERM_DATA; i-- > 0; )
	{
		int n;

		cptr s;

		/* Obtain */
		td = &data[i];

		/* Defaults */
		term_data_hack(td);

		/* Obtain title */
		s = angband_term_name[i];

		/* Get length */
		n = strlen(s);

		/* Maximal length */
		if (n > 15) n = 15;

		/* Copy the title */
		strncpy((char*)(td->title) + 1, s, n);

		/* Save the length */
		td->title[0] = n;

		/* Tile the windows */
		td->r.left += (b * 30);
		td->r.top += (b * 30);

		/* Tile */
		b++;
	}


	/*** Load preferences ***/

	/* Assume failure */
	oops = TRUE;

	/* Assume failure */
	fff = NULL;

#ifdef USE_SFL_CODE

	/* Block */
	if (TRUE)
	{
		OSErr	err;
		short vref;
		long dirID;
		char foo[128];

		/* Find the folder */
		err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
		                 &vref, &dirID);

		/* Success */
		if (!err)
		{
			/* Extract a path name */
			PathNameFromDirID(dirID, vref, (StringPtr)foo);

			/* Convert the string */
			ptocstr((StringPtr)foo);

			/* Append the preference file name */
			strcat(foo, ANGBAND_PREFERENCES);

			/* Open the preference file */
			fff = fopen(foo, "r");

			/* Success */
			oops = FALSE;
		}
	}

#endif /* USE_SFL_CODE */

#if 0 /* SysEnvirons? SetVol? I'd say no */

	/* Oops */
	if (oops)
	{
		SysEnvRec env;
		short savev;
		long saved;

		/* Save */
		HGetVol(0, &savev, &saved);

		/* Go to the "system" folder */
		SysEnvirons(curSysEnvVers, &env);
		SetVol(0, env.sysVRefNum);

		/* Open the file */
		fff = fopen(":Preferences:" ANGBAND_PREFERENCES, "r");
		if (!fff) fff = fopen(":" ANGBAND_PREFERENCES, "r");

		/* Restore */
		HSetVol(0, savev, saved);
	}

#endif /* relic */

	/* Load preferences */
	if (fff)
	{
		/* Load a real preference file */
		load_prefs();

		/* Close the file */
		my_fclose(fff);
	}


	/*** Instantiate ***/

	/* Main window */
	td = &data[0];

	/* Many keys */
	td->keys = 1024;

	/* Start visible */
	td->mapped = TRUE;

	/* Link (backwards, for stacking order) */
	for (i = MAX_TERM_DATA; i-- > 0; )
	{
		term_data_link(i);
	}

	/* Main window */
	td = &data[0];

	/* Main window */
	Term_activate(td->t);
}


/*
 * Exit the program
 */
static void save_pref_file(void)
{
	bool oops;


	/* Assume failure */
	oops = TRUE;

	/* Assume failure */
	fff = NULL;

	/* Text file */
	_ftype = 'TEXT';


#ifdef USE_SFL_CODE

	/* Block */
	if (TRUE)
	{
		OSErr	err;
		short vref;
		long dirID;
		char foo[128];

		/* Find the folder */
		err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
		                 &vref, &dirID);

		/* Success */
		if (!err)
		{
			/* Extract a path name */
			PathNameFromDirID(dirID, vref, (StringPtr)foo);

			/* Convert the string */
			ptocstr((StringPtr)foo);

			/* Append the preference file name */
			strcat(foo, ANGBAND_PREFERENCES);

			/* Open the preference file */
			fff = fopen(foo, "w");

			/* Success */
			oops = FALSE;
		}
	}

#endif /* USE_SFL_CODE */

#if 0 /* I don't believe we need SysEnvirons any longer */

	/* Oops */
	if (oops)
	{
		SysEnvRec env;
		short savev;
		long saved;

		/* Save */
		HGetVol(0, &savev, &saved);

		/* Go to "system" folder */
		SysEnvirons(curSysEnvVers, &env);
		SetVol(0, env.sysVRefNum);

		/* Open the preference file */
		fff = fopen(":Preferences:" ANGBAND_PREFERENCES, "w");
		if (!fff) fff = fopen(":" ANGBAND_PREFERENCES, "w");

		/* Restore */
		HSetVol(0, savev, saved);
	}

#endif /* relic */

	/* Save preferences */
	if (fff)
	{
		/* Write the preferences */
		save_prefs();

		/* Close it */
		my_fclose(fff);
	}
}



#ifdef ALLOW_NO_SAVE_QUITS

/*
 * A simple "Yes/No" filter to parse "key press" events in dialog windows
 */
static pascal Boolean ynfilter(DialogPtr dialog, EventRecord *event, short *ip)
{
	/* Parse key press events */
	if (event->what == keyDown)
	{
		int i = 0;
		char c;

		/* Extract the pressed key */
		c = (event->message & charCodeMask);

		/* Accept "no" and <return> and <enter> */
		if ((c == 'n') || (c == 'N') || (c == 13) || (c == 3)) i = 1;

		/* Accept "yes" */
		else if ((c == 'y') || (c == 'Y')) i = 2;

		/* Handle "yes" or "no" */
		if (i)
		{
			short type;
			ControlHandle control;
			Rect r;

			/* Get the button */
			GetDialogItem(dialog, i, &type, (Handle*)&control, &r);

			/* Blink button for 1/10 second */
			HiliteControl(control, 1);
			Term_xtra_mac(TERM_XTRA_DELAY, 100);
			HiliteControl(control, 0);

			/* Result */
			*ip = i;
			return (1);
		}
	}

	/* Ignore */
	return (0);
}

#endif /* ALLOW_NO_SAVE_QUITS */



#ifndef SAVEFILE_SCREEN

/*
 * Handle menu: "File" + "New"
 */
static void do_menu_file_new(void)
{
	/* Hack */
	HiliteMenu(0);

	/* Game is in progress */
	game_in_progress = 1;

	/* Flush input */
	Term_flush();

	/* Play a game */
	play_game(TRUE);

	/* Hack -- quit */
	quit(NULL);
}

/*
 * Handle menu: "File" + "Open"
 */
static void do_menu_file_open(bool all)
{
	int err;
	short vrefnum;
	long drefnum;
	long junk;
	DirInfo pb;
	SFTypeList types;
	SFReply reply;
	Point topleft;


	/* XXX XXX XXX */

	/* vrefnum = GetSFCurVol(); */
	vrefnum = -*((short*)0x214);

	/* drefnum = GetSFCurDir(); */
	drefnum = *((long*)0x398);

	/* Descend into "lib" folder */
	pb.ioCompletion = NULL;
	pb.ioNamePtr = "\plib";
	pb.ioVRefNum = vrefnum;
	pb.ioDrDirID = drefnum;
	pb.ioFDirIndex = 0;

	/* Check for errors */
	err = PBGetCatInfoSync((CInfoPBPtr) & pb);

	/* Success */
	if ((err == noErr) && (pb.ioFlAttrib & 0x10))
	{
		/* Descend into "lib/save" folder */
		pb.ioCompletion = NULL;
		pb.ioNamePtr = "\psave";
		pb.ioVRefNum = vrefnum;
		pb.ioDrDirID = pb.ioDrDirID;
		pb.ioFDirIndex = 0;

		/* Check for errors */
		err = PBGetCatInfoSync((CInfoPBPtr) & pb);

		/* Success */
		if ((err == noErr) && (pb.ioFlAttrib & 0x10))
		{
			/* SetSFCurDir(pb.ioDrDirID); */
			*((long*)0x398) = pb.ioDrDirID;
		}
	}

	/* Window location */
	topleft.h = (qd.screenBits.bounds.left + qd.screenBits.bounds.right) / 2 - 344 / 2;
	topleft.v = (2 * qd.screenBits.bounds.top + qd.screenBits.bounds.bottom) / 3 - 188 / 2;

	/* Allow "all" files */
	if (all)
	{
		/* Get any file */
		SFGetFile(topleft, "\p", NULL, -1, types, NULL, &reply);
	}

	/* Allow "save" files */
	else
	{
		/* Legal types */
		types[0] = 'SAVE';

		/* Get a file */
		SFGetFile(topleft, "\p", NULL, 1, types, NULL, &reply);
	}

	/* Allow cancel */
	if (!reply.good) return;

	/* Extract textual file name for save file */
	GetWDInfo(reply.vRefNum, &vrefnum, &drefnum, &junk);
	refnum_to_name(savefile, drefnum, vrefnum, (char*)reply.fName);

	/* Hack */
	HiliteMenu(0);

	/* Game is in progress */
	game_in_progress = 1;

	/* Flush input */
	flush();

	/* Play a game */
	play_game(FALSE);

	/* Hack -- quit */
	quit(NULL);
}

#endif /* !SAVEFILE_SCREEN */


/*
 * Handle the "open_when_ready" flag
 */
static void handle_open_when_ready(void)
{
	/* Check the flag XXX XXX XXX make a function for this */
	if (open_when_ready && initialized && !game_in_progress)
	{
		/* Forget */
		open_when_ready = FALSE;

		/* Game is in progress */
		game_in_progress = 1;

		/* Wait for it */
		pause_line(23);

		/* Flush input */
		flush();

#ifdef SAVEFILE_SCREEN

		/* User double-clicked savefile; no savefile screen */
		no_begin_screen = TRUE;

#endif /* SAVEFILE_SCREEN */

		/* Play a game */
		play_game(FALSE);

		/* Quit */
		quit(NULL);
	}
}



/*
 * The standard menus are:
 *
 *   Apple (128) =   { About, -, ... }
 *   File (129) =    { New,Open,Import,Close,Save,-,Score,Exit,Quit }
 *     In ToME, this becomes
 *   File (129) =    { Close,Save,-,Score,Exit,Quit }
 *   Edit (130) =    { Cut, Copy, Paste, Clear }   (?)
 *   Font (131) =    { Bold, Extend, -, Monaco, ..., -, ... }
 *   Size (132) =    { ... }
 *   Window (133) =  { Angband, Term-1/Mirror, Term-2/Recall, Term-3/Choice,
 *                     Term-4, Term-5, Term-6, Term-7 }
 *   Special (134) = { Sound, Graphics, TileWidth, TileHeight, -,
 *                     Fiddle, Wizard }
 */

/* Apple menu */
#define MENU_APPLE	128
#define ITEM_ABOUT	1

/* File menu */
#define MENU_FILE	129
#ifndef SAVEFILE_SCREEN
# define ITEM_NEW	1
# define ITEM_OPEN	2
# define ITEM_IMPORT	3
# define ITEM_CLOSE	4
# define ITEM_SAVE	5
# ifdef HAS_SCORE_MENU
# define ITEM_SCORE 7
# define ITEM_EXIT	8
# define ITEM_QUIT	9
# else
# define ITEM_EXIT	7
# define ITEM_QUIT	8
# endif  /* HAS_SCORE_MENU */
#else /* !SAVEFILE_SCREEN - in-game savefile menu */
# define ITEM_CLOSE	1
# define ITEM_SAVE	2
# ifdef HAS_SCORE_MENU
# define ITEM_SCORE	4
# define ITEM_EXIT	5
# define ITEM_QUIT	6
# else
# define ITEM_EXIT	4
# define ITEM_QUIT	5
# endif  /* HAS_SCORE_MENU */
#endif /* !SAVEFILE_SCREEN */

/* Edit menu */
#define MENU_EDIT	130
#define ITEM_UNDO	1
#define ITEM_CUT	3
#define ITEM_COPY	4
#define ITEM_PASTE	5
#define ITEM_CLEAR	6

/* Font menu */
#define MENU_FONT	131
#define ITEM_BOLD	1
#define ITEM_WIDE	2

/* Size menu */
#define MENU_SIZE	132

/* Windows menu */
#define MENU_WINDOWS	133

/* Special menu */
#define MENU_SPECIAL	134
#define ITEM_SOUND	1
#define ITEM_GRAPH	2
#define ITEM_TILEWIDTH 3
#define ITEM_TILEHEIGHT 4
#define ITEM_FIDDLE	6
#define ITEM_WIZARD	7

/* Graphics submenu */
#define SUBMENU_GRAPH	144
#define ITEM_NONE	1
#define ITEM_8X8	2
#define ITEM_16X16	3
#define ITEM_BIGTILE    5

/* TileWidth submenu */
#define SUBMENU_TILEWIDTH	145

/* TileHeight submenu */
#define SUBMENU_TILEHEIGHT	146


/*
 * Initialize the menus
 */
static void init_menubar(void)
{
	int i, n;

	Rect r;

	WindowPtr tmpw;

	MenuHandle m;

	Handle mbar;


	/* Load menubar from resources */
	mbar = GetNewMBar(128);

	/* Whoops! */
	if (mbar == nil) quit("Cannot find menubar('MBAR') id 128!");

	/* Insert them into the current menu list */
	SetMenuBar(mbar);

	/* Free handle */
	DisposeHandle(mbar);


	/* Apple menu (id 128) */
	m = GetMenuHandle(MENU_APPLE);

	/* Add the DA's to the "apple" menu */
	AppendResMenu(m, 'DRVR');


	/* File menu (id 129) - we don't have to do anything */


	/* Edit menu (id 130) - we don't have to do anything */


	/*
	 * Font menu (id 131) - append names of mono-spaced fonts
	 * followed by all available ones
	 */
	m = GetMenuHandle(MENU_FONT);

	/* Fake window */
	r.left = r.right = r.top = r.bottom = 0;

	/* Make the fake window so that we can retrive font info */
	tmpw = NewWindow(0, &r, "\p", false, documentProc, 0, 0, 0);

	/* Activate the "fake" window */
	SetPort(tmpw);

	/* Default mode */
	TextMode(0);

	/* Default size */
	TextSize(12);

	/* Add the fonts to the menu */
	AppendResMenu(m, 'FONT');

	/* Size of menu */
	n = CountMItems(m);

	/* Scan the menu */
	for (i = n; i >= 4; i--)
	{
		Str255 tmpName;
		short fontNum;

		/* Acquire the font name */
		GetMenuItemText(m, i, tmpName);

		/* Acquire the font index */
		GetFNum(tmpName, &fontNum);

		/* Apply the font index */
		TextFont(fontNum);

		/* Remove non-mono-spaced fonts */
		if ((CharWidth('i') != CharWidth('W')) || (CharWidth('W') == 0))
		{
			/* Delete the menu item */
			DeleteMenuItem(m, i);
		}
	}

	/* Destroy the old window */
	DisposeWindow(tmpw);

	/* Add a separator */
	AppendMenu(m, "\p-");

	/* Add the fonts to the menu */
	AppendResMenu(m, 'FONT');


	/* Size menu (id 132) */
	m = GetMenuHandle(MENU_SIZE);

	/* Add some sizes (stagger choices) */
	for (i = 8; i <= 32; i += ((i / 16) + 1))
	{
		Str15 buf;

		/* Textual size */
		strnfmt((char*)buf + 1, 15, "%d", i);
		buf[0] = strlen((char*)buf + 1);

		/* Add the item */
		AppendMenu(m, buf);
	}


	/* Windows menu (id 133) */
	m = GetMenuHandle(MENU_WINDOWS);

	/* Default choices */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		Str15 buf;

		/* Describe the item */
		strnfmt((char*)buf + 1, 15, "%.15s", angband_term_name[i]);
		buf[0] = strlen((char*)buf + 1);

		/* Add the item */
		AppendMenu(m, buf);

		/* Command-Key shortcuts */
		if (i < 8) SetItemCmd(m, i + 1, I2D(i));
	}


	/* Special menu (id 134) */

	/* Get graphics (sub)menu (id 144) */
	m = GetMenu(SUBMENU_GRAPH);

	/* Insert it as a submenu */
	InsertMenu(m, hierMenu);


	/* Get TileWidth (sub)menu (id 145) */
	m = GetMenu(SUBMENU_TILEWIDTH);

	/* Add some sizes */
	for (i = 4; i <= 32; i++)
	{
		Str15 buf;

		/* Textual size */
		strnfmt((char*)buf + 1, 15, "%d", i);
		buf[0] = strlen((char*)buf + 1);

		/* Append item */
		AppendMenu(m, buf);
	}

	/* Insert it as a submenu */
	InsertMenu(m, hierMenu);

	/* Get TileHeight (sub)menu (id 146) */
	m = GetMenu(SUBMENU_TILEHEIGHT);

	/* Add some sizes */
	for (i = 4; i <= 32; i++)
	{
		Str15 buf;

		/* Textual size */
		strnfmt((char*)buf + 1, 15, "%d", i);
		buf[0] = strlen((char*)buf + 1);

		/* Append item */
		AppendMenu(m, buf);
	}

	/* Insert it as a submenu */
	InsertMenu(m, hierMenu);


	/* Update the menu bar */
	DrawMenuBar();
}


/*
 * Prepare the menus
 *
 * It is very important that the player not be allowed to "save" the game
 * unless the "inkey_flag" variable is set, indicating that the game is
 * waiting for a new command.  XXX XXX XXX
 */
static void setup_menus(void)
{
	int i, n;

	short value;

	Str255 s;

	MenuHandle m;

	term_data *td = NULL;


	/* Relevant "term_data" */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Unused */
		if (!data[i].t) continue;

		/* Notice the matching window */
		if (data[i].w == FrontWindow()) td = &data[i];
	}


	/* File menu */
	m = GetMenuHandle(MENU_FILE);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

#ifndef SAVEFILE_SCREEN

	/* Enable "new"/"open..."/"import..." */
	if (initialized && !game_in_progress)
	{
		EnableItem(m, ITEM_NEW);
		EnableItem(m, ITEM_OPEN);
		EnableItem(m, ITEM_IMPORT);
	}

#endif /* !SAVEFILE_SCREEN */

	/* Enable "close" */
	if (initialized)
	{
		EnableItem(m, ITEM_CLOSE);
	}

	/* Enable "save" */
	if (initialized && character_generated && inkey_flag)
	{
		EnableItem(m, ITEM_SAVE);
	}

#ifdef HAS_SCORE_MENU

	/* Enable "score" */
	if (initialized && character_generated && !character_icky)
	{
		EnableItem(m, ITEM_SCORE);
	}

#endif /* HAS_SCORE_MENU */

#ifdef ALLOW_NO_SAVE_QUITS

	/* Enable "exit" */
	if (TRUE)
	{
		EnableItem(m, ITEM_EXIT);
	}

#endif /* ALLOW_NO_SAVE_QUITS */

	/* Enable "quit" */
	if (!initialized || !character_generated || inkey_flag)
	{
		EnableItem(m, ITEM_QUIT);
	}


	/* Edit menu */
	m = GetMenuHandle(MENU_EDIT);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

	/* Enable "edit" options if "needed" */
	if (!td)
	{
		EnableItem(m, ITEM_UNDO);
		EnableItem(m, ITEM_CUT);
		EnableItem(m, ITEM_COPY);
		EnableItem(m, ITEM_PASTE);
		EnableItem(m, ITEM_CLEAR);
	}


	/* Font menu */
	m = GetMenuHandle(MENU_FONT);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

	/* Hack -- look cute XXX XXX */
	/* SetItemStyle(m, ITEM_BOLD, bold); */

	/* Hack -- look cute XXX XXX */
	/* SetItemStyle(m, ITEM_WIDE, extend); */

	/* Active window */
	if (initialized && td)
	{
		/* Enable "bold" */
		EnableItem(m, ITEM_BOLD);

		/* Enable "extend" */
		EnableItem(m, ITEM_WIDE);

		/* Check the appropriate "bold-ness" */
		if (td->font_face & bold) CheckItem(m, ITEM_BOLD, TRUE);

		/* Check the appropriate "wide-ness" */
		if (td->font_face & extend) CheckItem(m, ITEM_WIDE, TRUE);

		/* Analyze fonts */
		for (i = 4; i <= n; i++)
		{
			/* Enable it */
			EnableItem(m, i);

			/* Analyze font */
			GetMenuItemText(m, i, s);
			GetFNum(s, &value);

			/* Check active font */
			if (td->font_id == value) CheckItem(m, i, TRUE);
		}
	}


	/* Size menu */
	m = GetMenuHandle(MENU_SIZE);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

	/* Active window */
	if (initialized && td)
	{
		/* Analyze sizes */
		for (i = 1; i <= n; i++)
		{
			/* Analyze size */
			GetMenuItemText(m, i, s);
			s[s[0] + 1] = '\0';
			value = atoi((char*)(s + 1));

			/* Enable the "real" sizes */
			if (RealFont(td->font_id, value)) EnableItem(m, i);

			/* Check the current size */
			if (td->font_size == value) CheckItem(m, i, TRUE);
		}
	}


	/* Windows menu */
	m = GetMenuHandle(MENU_WINDOWS);

	/* Get menu size */
	n = CountMItems(m);

	/* Check active windows */
	for (i = 1; i <= n; i++)
	{
		/* Check if needed */
		CheckItem(m, i, data[i - 1].mapped);
	}


	/* Special menu */
	m = GetMenuHandle(MENU_SPECIAL);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);

		/* XXX Oh no, this removes submenu... */
		if ((i != ITEM_GRAPH) &&
		                (i != ITEM_TILEWIDTH) &&
		                (i != ITEM_TILEHEIGHT)) CheckItem(m, i, FALSE);
	}

	/* Item "Sound" */
	EnableItem(m, ITEM_SOUND);
	CheckItem(m, ITEM_SOUND, arg_sound);

	/* Item "Graphics" */
	EnableItem(m, ITEM_GRAPH);

	/* Item "TileWidth" */
	EnableItem(m, ITEM_TILEWIDTH);

	/* Item "TileHeight" */
	EnableItem(m, ITEM_TILEHEIGHT);

	/* Item "Fiddle" */
	EnableItem(m, ITEM_FIDDLE);
	CheckItem(m, ITEM_FIDDLE, arg_fiddle);

	/* Item "Wizard" */
	EnableItem(m, ITEM_WIZARD);
	CheckItem(m, ITEM_WIZARD, arg_wizard);

	/* Graphics submenu */
	m = GetMenuHandle(SUBMENU_GRAPH);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

	/* Item "None" */
	EnableItem(m, ITEM_NONE);
	CheckItem(m, ITEM_NONE, (graf_mode == GRAF_MODE_NONE));

	/* Item "8x8" */
	EnableItem(m, ITEM_8X8);
	CheckItem(m, ITEM_8X8, (graf_mode == GRAF_MODE_8X8));

	/* Item "16x16" */
	EnableItem(m, ITEM_16X16);
	CheckItem(m, ITEM_16X16, (graf_mode == GRAF_MODE_16X16));

#ifdef USE_DOUBLE_TILES

	/* Item "Bigtile" */
	if (inkey_flag) EnableItem(m, ITEM_BIGTILE);
	CheckItem(m, ITEM_BIGTILE, use_bigtile);

#endif /* USE_DOUBLE_TILES */


	/* TIleWidth submenu */
	m = GetMenuHandle(SUBMENU_TILEWIDTH);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

	/* Active window */
	if (initialized && td)
	{
		/* Analyze sizes */
		for (i = 1; i <= n; i++)
		{
			/* Analyze size */
			GetMenuItemText(m, i, s);
			s[s[0] + 1] = '\0';
			value = atoi((char*)(s + 1));

			/* Enable */
			if (value >= td->font_wid) EnableItem(m, i);

			/* Check the current size */
			if (td->tile_wid == value) CheckItem(m, i, TRUE);
		}
	}


	/* TileHeight submenu */
	m = GetMenuHandle(SUBMENU_TILEHEIGHT);

	/* Get menu size */
	n = CountMItems(m);

	/* Reset menu */
	for (i = 1; i <= n; i++)
	{
		/* Reset */
		DisableItem(m, i);
		CheckItem(m, i, FALSE);
	}

	/* Active window */
	if (initialized && td)
	{
		/* Analyze sizes */
		for (i = 1; i <= n; i++)
		{
			/* Analyze size */
			GetMenuItemText(m, i, s);
			s[s[0] + 1] = '\0';
			value = atoi((char*)(s + 1));

			/* Enable */
			if (value >= td->font_hgt) EnableItem(m, i);

			/* Check the current size */
			if (td->tile_hgt == value) CheckItem(m, i, TRUE);
		}
	}
}


/*
 * Process a menu selection (see above)
 *
 * Hack -- assume that invalid menu selections are disabled above,
 * which I have been informed may not be reliable.  XXX XXX XXX
 */
static void menu(long mc)
{
	int i;

	int menuid, selection;

	static unsigned char s[1000];

	short fid;

	term_data *td = NULL;

	WindowPtr old_win;


	/* Analyze the menu command */
	menuid = HiWord(mc);
	selection = LoWord(mc);


	/* Find the window */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Skip dead windows */
		if (!data[i].t) continue;

		/* Notice matches */
		if (data[i].w == FrontWindow()) td = &data[i];
	}


	/* Branch on the menu */
	switch (menuid)
	{
		/* Apple Menu */
	case MENU_APPLE:
		{
			/* About Angband... */
			if (selection == ITEM_ABOUT)
			{
				DialogPtr dialog;
				Rect r;
				short item_hit;

				dialog = GetNewDialog(128, 0, (WindowPtr) - 1);

				r = dialog->portRect;
				center_rect(&r, &qd.screenBits.bounds);
				MoveWindow(dialog, r.left, r.top, 1);
				ShowWindow(dialog);
				ModalDialog(0, &item_hit);
				DisposeDialog(dialog);
				break;
			}

			/* Desk accessory */
			GetMenuItemText(GetMenuHandle(MENU_APPLE), selection, s);
			OpenDeskAcc(s);
			break;
		}


		/* File Menu */
	case MENU_FILE:
		{
			switch (selection)
			{
#ifndef SAVEFILE_SCREEN

				/* New */
			case ITEM_NEW:
				{
					do_menu_file_new();
					break;
				}

				/* Open... */
			case ITEM_OPEN:
				{
					do_menu_file_open(FALSE);
					break;
				}

				/* Import... */
			case ITEM_IMPORT:
				{
					do_menu_file_open(TRUE);
					break;
				}

#endif /* !SAVEFILE_SCREEN */

				/* Close */
			case ITEM_CLOSE:
				{
					/* No window */
					if (!td) break;

					/* Not Mapped */
					td->mapped = FALSE;

					/* Not Mapped */
					td->t->mapped_flag = FALSE;

					/* Hide the window */
					HideWindow(td->w);

					break;
				}

				/* Save */
			case ITEM_SAVE:
				{
					/* Hack -- Forget messages */
					msg_flag = FALSE;

					/* Hack -- Save the game */
#ifndef ZANG_AUTO_SAVE
					do_cmd_save_game();
#else
do_cmd_save_game(FALSE);
#endif /* !ZANG_AUTO_SAVE */

					break;
				}

#ifdef HAS_SCORE_MENU

				/* Show score */
			case ITEM_SCORE:
				{
					char buf[1024];

					/* Paranoia */
					if (!initialized || character_icky ||
					                !game_in_progress || !character_generated)
					{
						/* Can't happen but just in case */
						plog("You may not do that right now.");

						break;
					}

					/* Build the pathname of the score file */
					path_build(buf, sizeof(buf), ANGBAND_DIR_APEX,
					           "scores.raw");

					/* Hack - open the score file for reading */
					highscore_fd = fd_open(buf, O_RDONLY);

					/* Paranoia - No score file */
					if (highscore_fd < 0)
					{
						msg_print("Score file is not available.");

						break;
					}

					/* Mega-Hack - prevent various functions XXX XXX XXX */
					initialized = FALSE;

					/* Save screen */
					screen_save();

					/* Clear screen */
					Term_clear();

					/* Prepare scores */
					if (game_in_progress && character_generated)
					{
						predict_score();
					}

#if 0 /* I don't like this - pelpel */

					/* Mega-Hack - No current player XXX XXX XXX XXX */
					else
					{
						display_scores_aux(0, MAX_HISCORES, -1, NULL);
					}

#endif

					/* Close the high score file */
					(void)fd_close(highscore_fd);

					/* Forget the fd */
					highscore_fd = -1;

					/* Restore screen */
					screen_load();

					/* Hack - Flush it */
					Term_fresh();

					/* Mega-Hack - We are ready again */
					initialized = TRUE;

					/* Done */
					break;
				}

#endif /* HAS_SCORE_MENU */

#ifdef ALLOW_NO_SAVE_QUITS

				/* Exit (without save) */
			case ITEM_EXIT:
				{
					/* Allow user to cancel "dangerous" exit */
					if (game_in_progress && character_generated)
					{
						AlertTHndl alert;
						short item_hit;

						/* Get the "alert" info */
						alert = (AlertTHndl)GetResource('ALRT', 130);

						/* Center the "alert" rectangle */
						center_rect(&(*alert)->boundsRect,
						            &qd.screenBits.bounds);

						/* Display the Alert, get "No" or "Yes" */
						item_hit = Alert(130, ynfilterUPP);

						/* Require "yes" button */
						if (item_hit != 2) break;
					}

					/* Quit */
					quit(NULL);
					break;
				}

#endif /* ALLOW_NO_SAVE_QUITS */

				/* Quit (with save) */
			case ITEM_QUIT:
				{
					/* Save the game (if necessary) */
					if (game_in_progress && character_generated)
					{
						/* Hack -- Forget messages */
						msg_flag = FALSE;

						/* Save the game */
#ifndef ZANG_AUTO_SAVE
						do_cmd_save_game();
#else
						do_cmd_save_game(FALSE);
#endif /* !ZANG_AUTO_SAVE */
					}

					/* Quit */
					quit(NULL);
					break;
				}
			}

			break;
		}


		/* Edit menu */
	case MENU_EDIT:
		{
			/* Unused */
			break;
		}


		/* Font menu */
	case MENU_FONT:
		{
			/* Require a window */
			if (!td) break;

			/* Memorize old */
			old_win = active;

			/* Activate */
			activate(td->w);

			/* Toggle the "bold" setting */
			if (selection == ITEM_BOLD)
			{
				/* Toggle the setting */
				if (td->font_face & bold)
				{
					td->font_face &= ~bold;
				}
				else
				{
					td->font_face |= bold;
				}

				/* Hack - clear tile size info XXX XXX */
				td->tile_wid = td->tile_hgt = 0;

				/* Apply and Verify */
				term_data_check_font(td);
				term_data_check_size(td);

				/* Resize and Redraw */
				term_data_resize(td);
				term_data_redraw(td);

				break;
			}

			/* Toggle the "wide" setting */
			if (selection == ITEM_WIDE)
			{
				/* Toggle the setting */
				if (td->font_face & extend)
				{
					td->font_face &= ~extend;
				}
				else
				{
					td->font_face |= extend;
				}

				/* Hack - clear tile size info XXX XXX */
				td->tile_wid = td->tile_hgt = 0;

				/* Apply and Verify */
				term_data_check_font(td);
				term_data_check_size(td);

				/* Resize and Redraw */
				term_data_resize(td);
				term_data_redraw(td);

				break;
			}

			/* Get a new font name */
			GetMenuItemText(GetMenuHandle(MENU_FONT), selection, s);
			GetFNum(s, &fid);

			/* Save the new font id */
			td->font_id = fid;

			/* Current size is bad for new font */
			if (!RealFont(td->font_id, td->font_size))
			{
				/* Find similar size */
				for (i = 1; i <= 32; i++)
				{
					/* Adjust smaller */
					if (td->font_size - i >= 8)
					{
						if (RealFont(td->font_id, td->font_size - i))
						{
							td->font_size -= i;
							break;
						}
					}

					/* Adjust larger */
					if (td->font_size + i <= 128)
					{
						if (RealFont(td->font_id, td->font_size + i))
						{
							td->font_size += i;
							break;
						}
					}
				}
			}

			/* Hack - clear tile size info XXX XXX */
			td->tile_wid = td->tile_hgt = 0;

			/* Apply and Verify */
			term_data_check_font(td);
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore the window */
			activate(old_win);

			break;
		}

		/* Size menu */
	case MENU_SIZE:
		{
			if (!td) break;

			/* Save old */
			old_win = active;

			/* Activate */
			activate(td->w);

			GetMenuItemText(GetMenuHandle(MENU_SIZE), selection, s);
			s[s[0] + 1] = 0;
			td->font_size = atoi((char*)(s + 1));

			/* Hack - clear tile size info XXX XXX */
			td->tile_wid = td->tile_hgt = 0;

			/* Apply and Verify */
			term_data_check_font(td);
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore */
			activate(old_win);

			break;
		}

		/* Window menu */
	case MENU_WINDOWS:
		{
			/* Parse */
			i = selection - 1;

			/* Check legality of choice */
			if ((i < 0) || (i >= MAX_TERM_DATA)) break;

			/* Obtain the window */
			td = &data[i];

			/* Mapped */
			td->mapped = TRUE;

			/* Link */
			term_data_link(i);

			/* Mapped (?) */
			td->t->mapped_flag = TRUE;

			/* Show the window */
			ShowWindow(td->w);

			/* Bring to the front */
			SelectWindow(td->w);

			break;
		}

		/* Special menu */
	case MENU_SPECIAL:
		{
			switch (selection)
			{
			case ITEM_SOUND:
				{
					/* Toggle arg_sound */
					arg_sound = !arg_sound;

					/* React to changes */
					Term_xtra(TERM_XTRA_REACT, 0);

					break;
				}

			case ITEM_FIDDLE:
				{
					arg_fiddle = !arg_fiddle;
					break;
				}

			case ITEM_WIZARD:
				{
					arg_wizard = !arg_wizard;
					break;
				}
			}

			break;
		}


		/* Graphics submenu */
	case SUBMENU_GRAPH:
		{
			switch (selection)
			{
			case ITEM_NONE:
				{
					graf_mode_req = GRAF_MODE_NONE;

					break;
				}

			case ITEM_8X8:
				{
					graf_mode_req = GRAF_MODE_8X8;

					break;
				}

			case ITEM_16X16:
				{
					graf_mode_req = GRAF_MODE_16X16;

					break;
				}

#ifdef USE_DOUBLE_TILES

			case ITEM_BIGTILE:
				{
					term *old = Term;
					term_data *td = &data[0];

					/* Toggle "arg_bigtile" */
					use_bigtile = !use_bigtile;
# ifdef TOME
					arg_bigtile = use_bigtile;
# endif  /* TOME */

					/* Activate */
					Term_activate(td->t);

					/* Resize the term */
					Term_resize(td->cols, td->rows);

					/* Activate old */
					Term_activate(old);

					break;
				}

#endif /* USE_DOUBLE_TILES */

			}

			/* Hack -- Force redraw */
			Term_key_push(KTRL('R'));

			break;
		}


		/* TileWidth submenu */
	case SUBMENU_TILEWIDTH:
		{
			if (!td) break;

			/* Save old */
			old_win = active;

			/* Activate */
			activate(td->w);

			GetMenuItemText(GetMenuHandle(SUBMENU_TILEWIDTH), selection, s);
			s[s[0] + 1] = 0;
			td->tile_wid = atoi((char*)(s + 1));

			/* Apply and Verify */
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore */
			activate(old_win);

			break;
		}


		/* TileHeight submenu */
	case SUBMENU_TILEHEIGHT:
		{
			if (!td) break;

			/* Save old */
			old_win = active;

			/* Activate */
			activate(td->w);

			GetMenuItemText(GetMenuHandle(SUBMENU_TILEHEIGHT), selection, s);
			s[s[0] + 1] = 0;
			td->tile_hgt = atoi((char*)(s + 1));

			/* Apply and Verify */
			term_data_check_size(td);

			/* Resize and Redraw */
			term_data_resize(td);
			term_data_redraw(td);

			/* Restore */
			activate(old_win);

			break;
		}
	}


	/* Clean the menu */
	HiliteMenu(0);
}


#ifdef USE_SFL_CODE


/*
 * Check for extra required parameters -- From "Maarten Hazewinkel"
 */
static OSErr CheckRequiredAEParams(const AppleEvent *theAppleEvent)
{
	OSErr aeError;
	DescType returnedType;
	Size actualSize;

	aeError = AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr, typeWildCard,
	                            &returnedType, NULL, 0, &actualSize);

	if (aeError == errAEDescNotFound) return (noErr);

	if (aeError == noErr) return (errAEParamMissed);

	return (aeError);
}


/*
 * Apple Event Handler -- Open Application
 */
static OSErr AEH_Start(const AppleEvent *theAppleEvent,
                       AppleEvent *reply, long handlerRefCon)
{
#pragma unused(reply, handlerRefCon)

	return (CheckRequiredAEParams(theAppleEvent));
}


/*
 * Apple Event Handler -- Quit Application
 */
static OSErr AEH_Quit(const AppleEvent *theAppleEvent,
                      AppleEvent *reply, long handlerRefCon)
{
#pragma unused(reply, handlerRefCon)

	/* Quit later */
	quit_when_ready = TRUE;

	/* Check arguments */
	return (CheckRequiredAEParams(theAppleEvent));
}


/*
 * Apple Event Handler -- Print Documents
 */
static OSErr AEH_Print(const AppleEvent *theAppleEvent,
                       AppleEvent *reply, long handlerRefCon)
{
#pragma unused(theAppleEvent, reply, handlerRefCon)

	return (errAEEventNotHandled);
}


/*
 * Apple Event Handler by Steve Linberg (slinberg@crocker.com).
 *
 * The old method of opening savefiles from the finder does not work
 * on the Power Macintosh, because CountAppFiles and GetAppFiles,
 * used to return information about the selected document files when
 * an application is launched, are part of the Segment Loader, which
 * is not present in the RISC OS due to the new memory architecture.
 *
 * The "correct" way to do this is with AppleEvents.  The following
 * code is modeled on the "Getting Files Selected from the Finder"
 * snippet from Think Reference 2.0.  (The prior sentence could read
 * "shamelessly swiped & hacked")
 */
static OSErr AEH_Open(AppleEvent *theAppleEvent,
                      AppleEvent* reply, long handlerRefCon)
{
#pragma unused(reply, handlerRefCon)

	FSSpec myFSS;
	AEDescList docList;
	OSErr err;
	Size actualSize;
	AEKeyword keywd;
	DescType returnedType;
	char msg[128];
	FInfo myFileInfo;

	/* Put the direct parameter (a descriptor list) into a docList */
	err = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList);
	if (err) return err;

	/*
	 * We ignore the validity check, because we trust the FInder, and we only
	 * allow one savefile to be opened, so we ignore the depth of the list.
	 */

	err = AEGetNthPtr(&docList, 1L, typeFSS, &keywd,
	                  &returnedType, (Ptr) & myFSS, sizeof(myFSS), &actualSize);
	if (err) return err;

	/* Only needed to check savefile type below */
	err = FSpGetFInfo(&myFSS, &myFileInfo);
	if (err)
	{
		strnfmt(msg, 128, "Argh!  FSpGetFInfo failed with code %d", err);
		mac_warning(msg);
		return err;
	}

	/* Ignore non 'SAVE' files */
	if (myFileInfo.fdType != 'SAVE') return noErr;

	/* XXX XXX XXX Extract a file name */
	PathNameFromDirID(myFSS.parID, myFSS.vRefNum, (StringPtr)savefile);
	pstrcat((StringPtr)savefile, (StringPtr)&myFSS.name);

	/* Convert the string */
	ptocstr((StringPtr)savefile);

	/* Delay actual open */
	open_when_ready = TRUE;

	/* Dispose */
	err = AEDisposeDesc(&docList);

	/* Success */
	return noErr;
}


#endif



/*
 * Macintosh modifiers (event.modifier & ccc):
 *   cmdKey, optionKey, shiftKey, alphaLock, controlKey
 *
 *
 * Macintosh Keycodes (0-63 normal, 64-95 keypad, 96-127 extra):
 *
 * Return:36
 * Delete:51
 *
 * Period:65
 * Star:67
 * Plus:69
 * Clear:71
 * Slash:75
 * Enter:76
 * Minus:78
 * Equal:81
 * 0-7:82-89
 * 8-9:91-92
 *
 * backslash/vertical bar (Japanese keyboard):93
 *
 * F5: 96
 * F6: 97
 * F7: 98
 * F3:99
 * F8:100
 * F10:101
 * F11:103
 * F13:105
 * F14:107
 * F9:109
 * F12:111
 * F15:113
 * Help:114
 * Home:115
 * PgUp:116
 * Del:117
 * F4: 118
 * End:119
 * F2:120
 * PgDn:121
 * F1:122
 * Lt:123
 * Rt:124
 * Dn:125
 * Up:126
 */


/*
 * Optimize non-blocking calls to "CheckEvents()"
 * Idea from "Maarten Hazewinkel <mmhazewi@cs.ruu.nl>"
 */
#define EVENT_TICKS 6


/*
 * Check for Events, return TRUE if we process any
 *
 * Hack -- Handle AppleEvents if appropriate (ignore result code).
 */
static bool CheckEvents(bool wait)
{
	EventRecord event;

	WindowPtr w;

	Rect r;

	long newsize;

	int ch, ck;

	int mc, ms, mo, mx;

	int i;

	term_data *td = NULL;

	UInt32 curTicks;

	static UInt32 lastTicks = 0L;


	/* Access the clock */
	curTicks = TickCount();

	/* Hack -- Allow efficient checking for non-pending events */
	if (!wait && (curTicks < lastTicks + EVENT_TICKS)) return (FALSE);

	/* Timestamp last check */
	lastTicks = curTicks;

	/* Let the "system" run */
	SystemTask();

	/* Get an event (or null) */
	WaitNextEvent(everyEvent, &event, 0L, nil);

	/* Hack -- Nothing is ready yet */
	if (event.what == nullEvent) return (FALSE);


	/* Analyze the event */
	switch (event.what)
	{

#if 0

	case activateEvt:
		{
			w = (WindowPtr)event.message;

			activate(w);

			break;
		}

#endif

	case updateEvt:
		{
			/* Extract the window */
			w = (WindowPtr)event.message;

			/* Find the window */
			for (i = 0; i < MAX_TERM_DATA; i++)
			{
				/* Skip dead windows */
				if (!data[i].t) continue;

				/* Notice matches */
				if (data[i].w == w) td = &data[i];
			}

			/* Hack XXX XXX XXX */
			BeginUpdate(w);
			EndUpdate(w);

			/* Redraw the window */
			if (td) term_data_redraw(td);

			break;
		}

	case keyDown:
	case autoKey:
		{
			/* Extract some modifiers */
			mc = (event.modifiers & controlKey) ? TRUE : FALSE;
			ms = (event.modifiers & shiftKey) ? TRUE : FALSE;
			mo = (event.modifiers & optionKey) ? TRUE : FALSE;
			mx = (event.modifiers & cmdKey) ? TRUE : FALSE;

			/* Keypress: (only "valid" if ck < 96) */
			ch = (event.message & charCodeMask) & 255;

			/* Keycode: see table above */
			ck = ((event.message & keyCodeMask) >> 8) & 255;

			/* Command + "normal key" -> menu action */
			if (mx && (ck < 64))
			{
				/* Hack -- Prepare the menus */
				setup_menus();

				/* Mega-Hack -- allow easy exit if nothing to save */
				/* if (!character_generated && (ch=='Q' || ch=='q')) ch = 'e'; */

				/* Run the Menu-Handler */
				menu(MenuKey(ch));

				/* Turn off the menus */
				HiliteMenu(0);

				/* Done */
				break;
			}


			/* Hide the mouse pointer */
			ObscureCursor();

			/* Normal key -> simple keypress */
			if ((ck < 64) || (ck == 93))
			{
				/* Enqueue the keypress */
				Term_keypress(ch);
			}

			/* Keypad keys -> trigger plus simple keypress */
			else if (!mc && !ms && !mo && !mx && (ck < 96))
			{
				/* Hack -- "enter" is confused */
				if (ck == 76) ch = '\n';

				/* Begin special trigger */
				Term_keypress(31);

				/* Send the "keypad" modifier */
				Term_keypress('K');

				/* Send the "ascii" keypress */
				Term_keypress(ch);

				/* Terminate the trigger */
				Term_keypress(13);
			}

			/* Bizarre key -> encoded keypress */
			else if (ck <= 127)
			{
				/* Begin special trigger */
				Term_keypress(31);

				/* Send some modifier keys */
				if (mc) Term_keypress('C');
				if (ms) Term_keypress('S');
				if (mo) Term_keypress('O');
				if (mx) Term_keypress('X');

				/* Downshift and encode the keycode */
				Term_keypress(I2D((ck - 64) / 10));
				Term_keypress(I2D((ck - 64) % 10));

				/* Terminate the trigger */
				Term_keypress(13);
			}

			break;
		}

	case mouseDown:
		{
			int code;

			/* Analyze click location */
			code = FindWindow(event.where, &w);

			/* Find the window */
			for (i = 0; i < MAX_TERM_DATA; i++)
			{
				/* Skip dead windows */
				if (!data[i].t) continue;

				/* Notice matches */
				if (data[i].w == w) td = &data[i];
			}

			/* Analyze */
			switch (code)
			{
			case inMenuBar:
				{
					setup_menus();
					menu(MenuSelect(event.where));
					HiliteMenu(0);
					break;
				}

			case inSysWindow:
				{
					SystemClick(&event, w);
					break;
				}

			case inDrag:
				{
					Point p;

					WindowPtr old_win;

					r = qd.screenBits.bounds;
					r.top += 20;  /* GetMBarHeight() XXX XXX XXX */
					InsetRect(&r, 4, 4);
					DragWindow(w, event.where, &r);

					/* Oops */
					if (!td) break;

					/* Save */
					old_win = active;

					/* Activate */
					activate(td->w);

					/* Analyze */
					p.h = td->w->portRect.left;
					p.v = td->w->portRect.top;
					LocalToGlobal(&p);
					td->r.left = p.h;
					td->r.top = p.v;

					/* Restore */
					activate(old_win);

					/* Apply and Verify */
					term_data_check_size(td);

					break;
				}

			case inGoAway:
				{
					/* Oops */
					if (!td) break;

					/* Track the go-away box */
					if (TrackGoAway(w, event.where))
					{
						/* Not Mapped */
						td->mapped = FALSE;

						/* Not Mapped */
						td->t->mapped_flag = FALSE;

						/* Hide the window */
						HideWindow(td->w);
					}

					break;
				}

			case inGrow:
				{
					int x, y;

					term *old = Term;

					/* Oops */
					if (!td) break;

#ifndef ALLOW_BIG_SCREEN

					/* Fake rectangle */
					r.left = 20 * td->tile_wid + td->size_ow1;
					r.right = 80 * td->tile_wid + td->size_ow1 + td->size_ow2 + 1;
					r.top = 1 * td->tile_hgt + td->size_oh1;
					r.bottom = 24 * td->tile_hgt + td->size_oh1 + td->size_oh2 + 1;

#else /* ALLOW_BIG_SCREEN */

					/* Fake rectangle */
					r.left = 20 * td->tile_wid + td->size_ow1;
					r.right = qd.screenBits.bounds.right;
					r.top = 1 * td->tile_hgt + td->size_oh1;
					r.bottom = qd.screenBits.bounds.bottom;

#endif /* ALLOW_BIG_SCREEN */

					/* Grow the rectangle */
					newsize = GrowWindow(w, event.where, &r);

					/* Handle abort */
					if (!newsize) break;

					/* Extract the new size in pixels */
					y = HiWord(newsize) - td->size_oh1 - td->size_oh2;
					x = LoWord(newsize) - td->size_ow1 - td->size_ow2;

					/* Extract a "close" approximation */
					td->rows = y / td->tile_hgt;
					td->cols = x / td->tile_wid;

					/* Apply and Verify */
					term_data_check_size(td);

					/* Activate */
					Term_activate(td->t);

					/* Hack -- Resize the term */
					Term_resize(td->cols, td->rows);

					/* Resize and Redraw */
					term_data_resize(td);
					term_data_redraw(td);

					/* Restore */
					Term_activate(old);

					break;
				}

			case inContent:
				{
					SelectWindow(w);

					break;
				}
			}

			break;
		}

		/* Disk Event -- From "Maarten Hazewinkel" */
	case diskEvt:
		{
			/* check for error when mounting the disk */
			if (HiWord(event.message) != noErr)
			{
				Point p =
				        {120, 120};

				DILoad();
				DIBadMount(p, event.message);
				DIUnload();
			}

			break;
		}

		/* OS Event -- From "Maarten Hazewinkel" */
	case osEvt:
		{
			switch ((event.message >> 24) & 0x000000FF)
			{
			case suspendResumeMessage:

				/* Resuming: activate the front window */
				if (event.message & resumeFlag)
				{
					SetPort(FrontWindow());
					SetCursor(&qd.arrow);
				}

				/* Suspend: deactivate the front window */
				else
				{
					/* Nothing */
				}

				break;
			}

			break;
		}

#ifdef USE_SFL_CODE

		/* From "Steve Linberg" and "Maarten Hazewinkel" */
	case kHighLevelEvent:
		{
			/* Process apple events */
			if (AEProcessAppleEvent(&event) != noErr)
			{
				plog("Error in Apple Event Handler!");
			}

			/* Handle "quit_when_ready" */
			if (quit_when_ready)
			{
				/* Forget */
				quit_when_ready = FALSE;

				/* Do the menu key */
				menu(MenuKey('q'));

				/* Turn off the menus */
				HiliteMenu(0);
			}

			/* Handle "open_when_ready" */
			handle_open_when_ready();

			break;
		}

#endif

	}


	/* Something happened */
	return (TRUE);
}




/*** Some Hooks for various routines ***/


/*
 * Mega-Hack -- emergency lifeboat
 */
static void *lifeboat = NULL;


/*
 * Hook to "release" memory
 */
#ifdef NEW_ZVIRT_HOOKS /* [V] removed the unused 'size' argument. */
static void *hook_rnfree(void *v)
#else
static void *hook_rnfree(void *v, size_t size)
#endif /* NEW_ZVIRT_HOOKS */
{

#ifndef NEW_ZVIRT_HOOKS /* Oh, no. */
#pragma unused (size)
#endif

#ifdef USE_MALLOC

	/* Alternative method */
	free(v);

#else

	/* Dispose */
	DisposePtr(v);

#endif

	/* Success */
	return (NULL);
}

/*
 * Hook to "allocate" memory
 */
static void *hook_ralloc(size_t size)
{

#ifdef USE_MALLOC

	/* Make a new pointer */
	return (malloc(size));

#else

	/* Make a new pointer */
	return (NewPtr(size));

#endif

}

/*
 * Hook to handle "out of memory" errors
 */
static void *hook_rpanic(size_t size)
{

#pragma unused (size)

	/* void *mem = NULL; */

	/* Free the lifeboat */
	if (lifeboat)
	{
		/* Free the lifeboat */
		DisposePtr(lifeboat);

		/* Forget the lifeboat */
		lifeboat = NULL;

		/* Mega-Hack -- Warning */
		mac_warning("Running out of Memory!\rAbort this process now!");

		/* Mega-Hack -- Never leave this function */
		while (TRUE) CheckEvents(TRUE);
	}

	/* Mega-Hack -- Crash */
	return (NULL);
}


/*
 * Hook to tell the user something important
 */
static void hook_plog(cptr str)
{
	/* Warning message */
	mac_warning(str);
}


/*
 * Hook to tell the user something, and then quit
 */
static void hook_quit(cptr str)
{
	/* Warning if needed */
	if (str) mac_warning(str);

#ifdef USE_ASYNC_SOUND

	/* Cleanup sound player */
	cleanup_sound();

#endif /* USE_ASYNC_SOUND */

	/* Dispose of graphic tiles */
	if (frameP)
	{
		/* Unlock */
		BenSWUnlockFrame(frameP);

		/* Dispose of the GWorld */
		DisposeGWorld(frameP->framePort);

		/* Dispose of the memory */
		DisposePtr((Ptr)frameP);
	}

	/* Write a preference file */
	save_pref_file();

	/* All done */
	ExitToShell();
}


/*
 * Hook to tell the user something, and then crash
 */
static void hook_core(cptr str)
{
	/* XXX Use the debugger */
	/* DebugStr(str); */

	/* Warning */
	if (str) mac_warning(str);

	/* Warn, then save player */
	mac_warning("Fatal error.\rI will now attempt to save and quit.");

	/* Attempt to save */
	if (!save_player()) mac_warning("Warning -- save failed!");

	/* Quit */
	quit(NULL);
}



/*** Main program ***/


/*
 * Init some stuff
 *
 * XXX XXX XXX Hack -- This function attempts to "fix" the nasty
 * "Macintosh Save Bug" by using "absolute" path names, since on
 * System 7 machines anyway, the "current working directory" often
 * "changes" due to background processes, invalidating any "relative"
 * path names.  Note that the Macintosh is limited to 255 character
 * path names, so be careful about deeply embedded directories...
 *
 * XXX XXX XXX Hack -- This function attempts to "fix" the nasty
 * "missing lib folder bug" by allowing the user to help find the
 * "lib" folder by hand if the "application folder" code fails...
 */
static void init_stuff(void)
{
	int i;

	short vrefnum;
	long drefnum;
	long junk;

	SFTypeList types;
	SFReply reply;

	Rect r;
	Point topleft;

	char path[1024];


	/* Fake rectangle */
	r.left = 0;
	r.top = 0;
	r.right = 344;
	r.bottom = 188;

	/* Center it */
	center_rect(&r, &qd.screenBits.bounds);

	/* Extract corner */
	topleft.v = r.top;
	topleft.h = r.left;


	/* Default to the "lib" folder with the application */
	refnum_to_name(path, app_dir, app_vol, (char*)("\plib:"));


	/* Check until done */
	while (1)
	{
		/* Prepare the paths */
		init_file_paths(path);

		/* Build the filename */
		path_build(path, 1024, ANGBAND_DIR_FILE, "news.txt");

		/* Attempt to open and close that file */
		if (0 == fd_close(fd_open(path, O_RDONLY))) break;

		/* Warning */
		plog_fmt("Unable to open the '%s' file.", path);

		/* Warning */
		plog("The Angband 'lib' folder is probably missing or misplaced.");

		/* Warning */
		plog("Please 'open' any file in any sub-folder of the 'lib' folder.");

		/* Allow "text" files */
		types[0] = 'TEXT';

		/* Allow "save" files */
		types[1] = 'SAVE';

		/* Allow "data" files */
		types[2] = 'DATA';

		/* Get any file */
		SFGetFile(topleft, "\p", NULL, 3, types, NULL, &reply);

		/* Allow cancel */
		if (!reply.good) quit(NULL);

		/* Extract textual file name for given file */
		GetWDInfo(reply.vRefNum, &vrefnum, &drefnum, &junk);
		refnum_to_name(path, drefnum, vrefnum, (char*)reply.fName);

		/* Hack -- Remove the "filename" */
		i = strlen(path) - 1;
		while ((i > 0) && (path[i] != ':')) i--;
		if (path[i] == ':') path[i + 1] = '\0';

		/* Hack -- allow "lib" folders */
		if (suffix(path, "lib:")) continue;

		/* Hack -- Remove the "sub-folder" */
		i = i - 1;
		while ((i > 1) && (path[i] != ':')) i--;
		if (path[i] == ':') path[i + 1] = '\0';
	}
}


/*
 * Macintosh Main loop
 */
int main(void)
{
	int i;

	EventRecord tempEvent;
	int numberOfMasters = 10;

	/* Increase stack space by 64K */
	SetApplLimit(GetApplLimit() - 65536L);

	/* Stretch out the heap to full size */
	MaxApplZone();

	/* Get more Masters */
	while (numberOfMasters--) MoreMasters();

	/* Set up the Macintosh */
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	/* TEInit(); */
	InitDialogs(NULL);
	InitCursor();

	/* Flush events */
	FlushEvents(everyEvent, 0);

	/* Flush events some more (?) */
	if (EventAvail(everyEvent, &tempEvent)) FlushEvents(everyEvent, 0);


#ifdef ANGBAND_LITE_MAC

	/* Nothing */

#else /* ANGBAND_LITE_MAC */

# if defined(powerc) || defined(__powerc)

	/* Assume System 7 */

	/* Assume Color Quickdraw */

# else

	/* Block */
	if (TRUE)
	{
		OSErr err;
		long versionNumber;

		/* Check the Gestalt */
		err = Gestalt(gestaltSystemVersion, &versionNumber);

		/* Check the version */
		if ((err != noErr) || (versionNumber < 0x0700))
		{
			quit("You must have System 7 to use this program.");
		}
	}

	/* Block */
	if (TRUE)
	{
		SysEnvRec env;

		/* Check the environs */
		if (SysEnvirons(1, &env) != noErr)
		{
			quit("The SysEnvirons call failed!");
		}

		/* Check for System Seven Stuff */
		if (env.systemVersion < 0x0700)
		{
			quit("You must have System 7 to use this program.");
		}

		/* Check for Color Quickdraw */
		if (!env.hasColorQD)
		{
			quit("You must have Color Quickdraw to use this program.");
		}
	}

# endif 

#endif /* ANGBAND_LITE_MAC */


#ifdef USE_SFL_CODE

	/* Obtain a "Universal Procedure Pointer" */
	AEH_Start_UPP = NewAEEventHandlerProc(AEH_Start);

	/* Install the hook (ignore error codes) */
	AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, AEH_Start_UPP,
	                      0L, FALSE);

	/* Obtain a "Universal Procedure Pointer" */
	AEH_Quit_UPP = NewAEEventHandlerProc(AEH_Quit);

	/* Install the hook (ignore error codes) */
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, AEH_Quit_UPP,
	                      0L, FALSE);

	/* Obtain a "Universal Procedure Pointer" */
	AEH_Print_UPP = NewAEEventHandlerProc(AEH_Print);

	/* Install the hook (ignore error codes) */
	AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, AEH_Print_UPP,
	                      0L, FALSE);

	/* Obtain a "Universal Procedure Pointer" */
	AEH_Open_UPP = NewAEEventHandlerProc(AEH_Open);

	/* Install the hook (ignore error codes) */
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, AEH_Open_UPP,
	                      0L, FALSE);

#endif


	/* Find the current application */
	SetupAppDir();


	/* Mark ourself as the file creator */
	_fcreator = ANGBAND_CREATOR;

	/* Default to saving a "text" file */
	_ftype = 'TEXT';


#if defined(ALLOW_NO_SAVE_QUITS) && defined(__MWERKS__)

	/* Obtian a "Universal Procedure Pointer" */
	ynfilterUPP = NewModalFilterProc(ynfilter);

#endif /* ALLOW_NO_SAVE_QUITS && __MWERKS__ */


	/* Hook in some "z-virt.c" hooks */
	rnfree_aux = hook_rnfree;
	ralloc_aux = hook_ralloc;
	rpanic_aux = hook_rpanic;

	/* Hooks in some "z-util.c" hooks */
	plog_aux = hook_plog;
	quit_aux = hook_quit;
	core_aux = hook_core;


	/* Initialize colors */
	for (i = 0; i < 256; i++)
	{
		u16b rv, gv, bv;

		/* Extract the R,G,B data */
		rv = angband_color_table[i][1];
		gv = angband_color_table[i][2];
		bv = angband_color_table[i][3];

		/* Save the actual color */
		color_info[i].red = (rv | (rv << 8));
		color_info[i].green = (gv | (gv << 8));
		color_info[i].blue = (bv | (bv << 8));
	}


	/* Show the "watch" cursor */
	SetCursor(*(GetCursor(watchCursor)));

	/* Prepare the menubar */
	init_menubar();

	/* Prepare the windows */
	init_windows();

	/* Hack -- process all events */
	while (CheckEvents(TRUE)) /* loop */;

	/* Reset the cursor */
	SetCursor(&qd.arrow);


	/* Mega-Hack -- Allocate a "lifeboat" */
	lifeboat = NewPtr(16384);

	/* Note the "system" */
	ANGBAND_SYS = "mac";


	/* Initialize */
	init_stuff();

	/* Initialize */
	init_angband();


	/* Hack -- process all events */
	while (CheckEvents(TRUE)) /* loop */;


	/* We are now initialized */
	initialized = TRUE;


	/* Handle "open_when_ready" */
	handle_open_when_ready();

#ifndef SAVEFILE_SCREEN

	/* Prompt the user */
	/* In [Z], it's currently prtf(17, 23, <msg>); */
	prt("[Choose 'New' or 'Open' from the 'File' menu]", 23, 15);

	/* Flush the prompt */
	Term_fresh();

	/* Hack -- Process Events Forever */
	while (TRUE) CheckEvents(TRUE);

#else

	/* Game is in progress */
	game_in_progress = 1;

	/* Wait for keypress */
	pause_line(23);

	/* flush input - Warning: without this, _system_ would hang */
	flush();

	/* Play the game - note the value of the argument */
	play_game(FALSE);

	/* Quit */
	quit(NULL);

#endif /* !SAVEFILE_SCREEN */
}

#endif /* MACINTOSH */

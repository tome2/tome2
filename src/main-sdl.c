/* Copyright (C) 2003-2004 Neil Stevens <neil@hakubi.us>
 // Copyright (C) 2004 Ethan Stump <estump@seas.upenn.edu>
 //
 // Permission is hereby granted, free of charge, to any person obtaining a copy
 // of this software and associated documentation files (the "Software"), to deal
 // in the Software without restriction, including without limitation the rights
 // to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 // copies of the Software, and to permit persons to whom the Software is
 // furnished to do so, subject to the following conditions:
 //
 // The above copyright notice and this permission notice shall be included in
 // all copies or substantial portions of the Software.
 //
 // THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 // IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 // FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 // THE AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 // AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 // CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 //
 // Except as contained in this notice, the name(s) of the author(s) shall not be
 // used in advertising or otherwise to promote the sale, use or other dealings
 // in this Software without prior written authorization from the author(s).
 */

#ifdef USE_SDL

#include "angband.h"
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <math.h>

#ifdef USE_ISO
/*
 * Simugraph system (Hj. Malthaner)
 */
#include "iso/simsys.h"
#include "iso/simgraph.h"
#include "iso/world_adaptor.h"
#include "iso/world_view.h"
/*
 * Simugraph specific routines
 * by Hj. Malthaner
 */
#include "iso/hackdef.h"

/*
 * Text place marker function protype (Hj. Malthaner)
 */
static void set_spots(int x, int y, int n, bool v);

/**
 * we need to track spots with text to avoid overdrawing text with images
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
bool spots[80][24];

/**
 * mouse coordinates for Simugraph engine 
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
int mx, my;

/*
 * Hajo: this flags need to be set when opening windows
 */

static int tex_width;
static int tex_height;
static int tex_xoff;
static int tex_yoff;

static unsigned int tab16[1 << 16];

/**
 * this is used if we need to fake an 8 bit array
 * @author Hj. Malthaner
 */
static unsigned short * data8;

// buffers to store char code/attr for graphics
unsigned char **iso_ap;
unsigned char **iso_cp;
unsigned char **iso_atp;
unsigned char **iso_ctp;
unsigned char **iso_aep;
unsigned char **iso_cep;

#endif /* USE_ISO */

/*************************************************
 GLOBAL SDL-ToME PROPERTIES
 *************************************************/
 
/* Default window properties - used if none are available
from other places*/
#define DEF_SCREEN_WIDTH  800
#define DEF_SCREEN_HEIGHT 600
#define DEF_SCREEN_BPP     16

/*Main window properties that may be loaded at runtime from
a preference file or environmental variables. However,
default values (defined above) can be used. */
static int arg_width = DEF_SCREEN_WIDTH;
static int arg_height = DEF_SCREEN_HEIGHT;
static int arg_bpp = DEF_SCREEN_BPP;

/**************/

/* Default font properties - used unless otherwise changed.
These properties are the size and also default font to load. */
#define DEF_FONT_SIZE 14
#define DEF_FONT_FILE "VeraMono.ttf"

/* The font properties that may perhaps be changed at runtime,
due to environmental variables, preference files, or in-program
commands.*/
static int arg_font_size = DEF_FONT_SIZE;
static char arg_font_name[64] = DEF_FONT_FILE;

/**************/

/* Graphics setting - signifies what graphics to use. Valid ints
are available with given defines */

/* No graphics - use only colored text */
#define NO_GRAPHICS		0
/* "Old" graphics - use 8x8.bmp to extract graphics tiles */
#define GRAPHICS_8x8	8
/* "New" graphics - use 16x16.bmp as tiles and apply mask.bmp for transparency*/
#define GRAPHICS_16x16	16

static int arg_graphics_type = NO_GRAPHICS;


/**************/

/* The number of term_data structures to set aside mem for */
#define MAX_CONSOLE_COUNT 8

/* The number of consoles that are actually being used.
This number could be changed via preference files, environmental
variables, command-line arguments, or possibly even in-game
keypresses or menu-selections. */
static int arg_console_count = 1;

/* When rendering multiple terminals, each is drawn with a
surrounding border. This value controls the width of this
border */
#define BORDER_THICKNESS 1

/**************/

/* some miscellaneous settings which have not been dealt
with yet */
static bool arg_double_width = FALSE;

/* flag signifying whether the game is in full screen */
static bool arg_full_screen = FALSE;

/* a flag to show whether window properties have been
set or not... if so, the properties can be dumped
upon quit*/
static bool window_properties_set = FALSE;

/*************************************************
 GLOBAL SDL-ToME VARIABLES
 *************************************************/

/* the main screen to draw to */
static SDL_Surface *screen;

/* the video settings for the system */
static SDL_VideoInfo *videoInfo;

/* a flag to suspend updating of the screen;
this is in place so that when a large area is being
redrawn -- like when doing a Term_redraw() or when
redoing the entire screen -- all of the changes
can be stored up before doing an update. This
should cut down on screen flicker */
static bool suspendUpdate = FALSE;

/* some helper surfaces that are used for rendering 
characters */
static SDL_Surface *worksurf;
static SDL_Surface *crayon;

/* the cursor surface */
static SDL_Surface *cursor = NULL;

/* the array of pre-rendered characters
(see loadAndRenderFont() below) */
SDL_Surface *text[128];

/* the actual TTF_Font used (XXX should get rid of this)*/
TTF_Font *font=0;

/* the width and height of the uniformly-sized pre-rendered
characters */
int t_width = 0, t_height = 0;


/*************************************************
 COLOR SETUP
 *************************************************/
 
/* Simple black, mapped using the format of the main screen */
int screen_black;

/* The color to use for the cursor */
static int cursor_color = 0;
/* default cursor color is a semi-transparent yellow */
#define DEF_CURSOR_COLOR	255,255,0,128

/* The array of colors, mapped to the format of the crayon surface,
since this is ultimately the surface that color is begin applied to */
static int color_data[16];

/* The following macro is for color defining...
 Note that the color is fully opaque... */
#define COLOR(r,g,b) \
	SDL_MapRGBA(crayon->format,r,g,b,SDL_ALPHA_OPAQUE)

/*These color macros will setup the colors to use, but must be called after
 the SDL video has been set. That way SDL can correct for any funky video
 setttings. */

#define BLACK			COLOR(  0,  0,  0)	/* 0*/
#define WHITE			COLOR(255,255,255)	/* 1*/
#define MID_GREY		COLOR(128,128,128)	/* 2*/
#define BRIGHT_ORANGE	COLOR(255,128,  0)	/* 3*/
#define RED				COLOR(192,  0,  0)	/* 4*/
#define GREEN			COLOR(  0,128, 64)	/* 5*/
#define BRIGHT_BLUE		COLOR(  0,  0,255)	/* 6*/
#define DARK_ORANGE		COLOR(128, 64,  0)	/* 7*/
#define DARK_GREY		COLOR( 64, 64, 64)	/* 8*/
#define BRIGHT_GREY		COLOR(192,192,192)	/* 9*/
#define PURPLE			COLOR(255,  0,255)	/*10*/
#define YELLOW			COLOR(255,255,  0)	/*11*/
#define BRIGHT_RED		COLOR(255,  0,  0)	/*12*/
#define BRIGHT_GREEN	COLOR(  0,255,  0)	/*13*/
#define AQUAMARINE		COLOR(  0,255,255)	/*14*/
#define BROWN			COLOR(192,128, 64)	/*15*/

/*************************************************
 TERMINAL DATA STRUCTURE SETUP
 *************************************************/

/* Forward declare */
typedef struct _term_data term_data;

/* A structure for each "term" */
struct _term_data
{
	term t;					/* the term structure, defined in z-term.h */
	cptr name;				/* name of this term sub-window */

	uint rows, cols;		/* row/column count */
	SDL_Rect rect;			/* the bounding rectangle for the entire box;
								includes border and empty space as well */
							/* this rectangle is in screen coordinates */

	int border_thick;		/* thickness of border to draw around window */
	int border_color;		/* current color of the border */	
	uint cushion_x_top, cushion_x_bot, cushion_y_top, cushion_y_bot;
							/* empty space cushion between border and tiles */
	
	uint tile_width;		/* the width of each tile (graphic or otherwise)*/
	uint tile_height;		/* the height of each tile (graphic or otherwise)*/

	SDL_Surface	*surf;		/* the surface that graphics for this screen are
								rendered to before blitting to main screen */	
	int black,white,purple;	/* basic colors keyed to this terminal's surface */

#ifdef USE_GRAPHICS
#ifdef USE_TRANSPARENCY
#endif
#endif
};

/* The array of term data structures */
static term_data data[MAX_CONSOLE_COUNT];

/* Ordered array of pointers to term data structures, placed in order of
priority: lowest is on top of all others, the higher the index, the further
back into the screen that it is drawn */
static term_data *term_order[MAX_CONSOLE_COUNT];

/*************************************************
 FILE-SPECIFIC MACROS
 *************************************************/

/* Debug macros! */
#define DB(str) \
	printf("main-sdl: %s\n",str);

/* Prints out the RGBA values of a given color */
#define TYPECOLOR32(color) 	printf("  R:%d\tG:%d\tB:%d\tA:%d\t\n",\
	color>>24,(color&0x00ff0000)>>16,\
	(color&0x0000ff00)>>8,(color&0x000000ff))

#define TYPECOLOR16(color) 	printf("  R:%d\tG:%d\tB:%d\tA:%d\t\n",\
	(color&0xf000)>>12,(color&0x0f00)>>8,\
	(color&0x00f0)>>4,(color&0x000f))

/* SDL Surface locking and unlocking */
#define SDL_LOCK(surf) \
	if (SDL_MUSTLOCK(surf) ){ \
		if (SDL_LockSurface(surf) < 0) { \
			printf("Can't lock the screen: %s\n", SDL_GetError()); \
			exit(1); \
		} \
	}

#define SDL_UNLOCK(surf) \
	if (SDL_MUSTLOCK(surf) ){ \
		SDL_UnlockSurface(surf); \
	}

/* Wrapped SDL_UpdateRects function, to take into
account the fact that updates may be suspended by
the suspendUpdate flag... this macro should be used
whenever a rect needs updated */
#define SDL_UPDATE(rect) \
	if (!suspendUpdate) \
		SDL_UpdateRects(screen,1,&rect)

/* A complete screen redraw macro */
#define SDL_REDRAW_SCREEN \
	SDL_UpdateRect(screen,0,0,arg_width,arg_height)
	
/*************************************************
 QUITTING
 *************************************************/

/* function prototype */
void dumpWindowSettings(void);
	
/* SDL Quitting function... declare a few functions first.*/
void killFontAndAlphabet(void);
static void sdl_quit(cptr string)
{
	printf("sdl_quit called.\n");
	printf("message: %s\n",string);
	/* Need to take care of font and rendered characters */
	killFontAndAlphabet();
	if (TTF_WasInit())
		TTF_Quit();
	/* Then exit SDL */
	SDL_Quit();
	
	/* Dump the window properties, if available */
	if (window_properties_set)
		dumpWindowSettings();
	
	/* And now for the default quit behavior */
	quit_aux = 0;
	quit(string);
}

/*************************************************
 FONT SUPPORT FUNCTIONS
 *************************************************/

/* function prototype for creating surfaces */
SDL_Surface *createSurface(int width, int height);

/* killFontAndAlphabet will effectively de-initialize the font system;
it does this by closing the font and destroying any pre-rendered
text in memory */
void killFontAndAlphabet(void)
{
	int i;
	/* need to close a font and free all of its corresponding pre-rendered
	surfaces */
	if (font)
	{
		TTF_CloseFont(font);
		font = 0;
	}
	for (i=0;i<128;i++)
	{
		if(text[i])
		{
			SDL_FreeSurface(text[i]);
			text[i] = NULL;
		}
	}
}	

/* loadAndRenderFont is responsible for loading and initializing
a font. First, SDL_ttf calls are made to load and set the style
for the desired font. Next, a character alphabet is rendered and
each character is placed onto a uniformly-sized surface within
the text[] array. Whenever text is needed for displaying on-screen,
this array is referenced and the desired character picture is used. */
void loadAndRenderFont(char *fname, int size)
{
	int minx,maxx,miny,maxy,advance,i,midline = 0;
	char filename[PATH_MAX + 1];
	char fontdir[PATH_MAX + 1];
	SDL_Color base_color = {255,255,255,255};
	SDL_Surface *temp_surf;
	SDL_Rect tgt = {0,0,0,0};


	/* Assuming that the filename is valid,open the font */
	path_build(fontdir, PATH_MAX, ANGBAND_DIR_XTRA, "font");
	path_build(filename, PATH_MAX, fontdir, fname);
	font = TTF_OpenFont(filename,size);
	if (font == NULL)
		sdl_quit("Error loading that font!");
	/* Set the font style to normal */
	TTF_SetFontStyle(font,TTF_STYLE_NORMAL);

	/* Collect some measurements on this font -
	arbitrarily choose the letter 'a' to get width*/
	TTF_GlyphMetrics(font,'a',&minx,&maxx,&miny,&maxy,&advance);
	/* the width of each character tile */
	t_width = advance;
	/* the height of each character tile */
	t_height = TTF_FontHeight(font);
	/* position of the y=0 line in each tile */
	midline = TTF_FontAscent(font);
	
	/* now... render each of the individual characters */
	for (i=0;i<128;i++)
	{
		/* make a pretty blended glyph */
		temp_surf=TTF_RenderGlyph_Blended(font,i,base_color);
		/* and make sure that we got it right! */
		if (temp_surf == NULL)
			sdl_quit("Glyph failed to render!");
		/* get the metrics of this particular glyph so we can position it */
		TTF_GlyphMetrics(font,i,&minx,&maxx,&miny,&maxy,&advance);
		/* copy rendered glyph into text queue, at the right position*/
		tgt.x = minx;
		tgt.y = midline-maxy;
		/* but first... we'll need a surface in the text queue to blit to! */
		text[i] = createSurface(t_width,t_height);
		/* turn OFF src-alpha... results in brute
		 copy of the RGBA contents of surf */
		SDL_SetAlpha(temp_surf,0,0);
		SDL_BlitSurface(temp_surf,NULL,text[i],&tgt);
		/* turn OFF src-alpha since we'll be using worksurf for blitting */
		SDL_SetAlpha(text[i],0,0);
		/* kill the surface to patch up memory leaks */
		SDL_FreeSurface(temp_surf);
	}
}

#ifdef USE_ISO
/*************************************************
 ISO SUPPORT FUNCTIONS
 *************************************************/
/**
 * inits operating system stuff
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
int dr_os_init(int n, int *parameter)
{
	// Hajo:
	// unused in isov-x11
	return TRUE;
}


/**
 * opens graphics device/context/window of size w*h
 * @param w width
 * @param h height
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
int dr_os_open(int w, int h)
{
	const int left = 13;

/*	tex_width = (data[0].fd->dw * (data[0].t.wid - left + 2) + 3) & 0xFFFC;
	tex_height = data[0].fd->dh * (data[0].t.hgt - 2);

	tex_xoff = data[0].fd->dw * left;
	tex_yoff = data[0].fd->dh * 1 + 1; 
*/

	//tex_width = (data[0].size_w - (left - 2) * (data[0].size_w / data[0].cols) + 3) & 0xFFFC;
	// this is too big (but works :-))
	tex_width = data[0].size_w & 0xfffc;
	tex_height = (data[0].size_h / data[0].rows) * (data[0].rows-2);

	tex_xoff = (tex_width / data[0].cols) * left;
	tex_yoff = (tex_height / data[0].rows) * 1 + 1;
	
	return TRUE;
}


/**
 * closes operating system stuff
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
int dr_os_close()
{
	// Hajo:
	// unused in isov-x11
	return TRUE;
}


/**
 * retrieve display width
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
int dr_get_width()
{
	return data[0].size_w;
}


/**
 * retrieve display height
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
int dr_get_height()
{
	return data[0].size_h;
}


/**
 * this is used if we need to fake an 8 bit array
 * @author Hj. Malthaner
 */
static unsigned short * data8;


/**
 * creates a (maybe virtual) array of graphics data
 * @author Hj. Malthaner
 */
unsigned short * dr_textur_init()
{
	int i;

	printf("isov-sdl::dr_textur_init()\n");
	printf(" width  = %d\n", data[0].size_w);
	printf(" height = %d\n", data[0].size_h);

	for (i = 0; i < (1 << 16); i++)
	{
		// FIXME!!!
		// must consider color bits, or breaks in anything else but RGB 555
		unsigned int R;
		unsigned int G;
		unsigned int B;

		// RGB 555
		R = (i & 0x7C00) >> 10;
		G = (i & 0x03E0) >> 5;
		B = (i & 0x001F) >> 0;


		tab16[i] = SDL_MapRGB(screen->format, R << 3, G << 3, B << 3);
	}



	data8 = malloc((data[0].size_w) * (data[0].size_h) * 2);

	printf(" textur = %p\n", data8);

	// fake an 16 bit array and convert data before displaying
	return data8;
}

static void flush_area(int dest_x, int dest_y,
                       int x, int y, int w, int h)
{
	SDL_Surface *face = screen;

	if (SDL_LockSurface(face) == 0)
	{
		int i, j;
		const int bpp = screen->format->BytesPerPixel;

		for (j = 0; j < h; j++)
		{
			unsigned short * p = data8 + (y + j) * tex_width + x;
			unsigned char * row = face->pixels + (dest_y + j) * face->pitch + dest_x * bpp;


			for (i = 0; i < w; i++)
			{
				*((unsigned short*)row) = tab16[ *p++ ];
				row += bpp;
			}
		}
		SDL_UnlockSurface(face);
	}
}

/**
 * displays the array of graphics data
 * @author Hj. Malthaner
 */
void dr_textur(int xp, int yp, int w, int h)
{
	int y;

	// clipping unten
	if (yp + h > tex_height)
	{
		h = tex_height - yp;
	}

	/* debug spots
	for(y=0; y<24; y++) {
	int x;

	for(x=0; x<80; x++) {
	 if(spots[x][y]) {
	printf("X");
	 } else {
	printf(".");
	 }
}
	printf("\n");
}
	*/

	for (y = 0; y < SCREEN_HGT; y++)
	{
		const int left = 13;
		const int y1 = y + 1;
		int x = 0;

		yp = data[0].size_h / data[0].rows * y;

		spots[79][y1] = FALSE;

		do
		{
			int n = 0;
			while (x + n + left < 80 && !spots[x + n + left][y1])
			{
				n++;
			}

			xp = data[0].size_w / data[0].cols * x;


			flush_area(tex_xoff + xp, tex_yoff + yp,
			           xp, yp,
			           data[0].size_w / data[0].cols*(n),
				   data[0].size_h / data[0].rows);

			x += n;

			while (x + left < 80 && spots[x + left][y1])
			{
				x++;
			}
		}
		while (x + left < 80);
	}
}


/**
 * use this method to flush graphics pipeline (undrawn stuff) onscreen.
 * @author Hj. Malthaner
 */
void dr_flush()
{
	// Iso-view for angband needs no sync.
	// XSync(md,FALSE);
}


/**
 * set colormap entries
 * @author Hj. Malthaner
 */
void dr_setRGB8multi(int first, int count, unsigned char * data)
{
	// Hajo:
	// unused in isov-x11
}


/**
 * display/hide mouse pointer
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
void show_pointer(int yesno)
{
	// Hajo:
	// unused in isov-x11
}


/**
 * move mouse pointer
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
void move_pointer(int x, int y)
{
	// Hajo:
	// unused in isov-x11
}


/**
 * update softpointer position
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
void ex_ord_update_mx_my()
{
	// Hajo:
	// unused in isov-x11
}


/**
 * get events from the system
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
void GetEvents()
{
	// Hajo:
	// unused in isov-x11
}


/**
 * get events from the system without waiting
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
void GetEventsNoWait()
{
	// Hajo:
	// unused in isov-x11
}


/**
 * @returns time since progrma start in milliseconds
 * @author Hj. Malthaner
 */
long long dr_time(void)
{
	// Hajo:
	// unused in isov-x11
	return 0;
}


/**
 * sleeps some microseconds
 * @author Hj. Malthaner
 */
void dr_sleep(unsigned long usec)
{
	// Hajo:
	// unused in isov-x11
}


/**
 * loads a sample
 * @return a handle for that sample or -1 on failure
 * @author Hj. Malthaner
 */
int dr_load_sample(const char *filename)
{
	// Hajo:
	// unused in isov-x11
	return TRUE;
}


/**
 * plays a sample
 * @param key the key for the sample to be played
 * @author Hj. Malthaner
 */
void dr_play_sample(int key, int volume)
{
	// Hajo:
	// unused in isov-x11
}

static unsigned char ** halloc(int w, int h)
{
	unsigned char **field = (unsigned char **)malloc(sizeof(unsigned char *) * h);
	int i;

	for (i = 0; i < h; i++)
	{
		field[i] = (unsigned char *)malloc(sizeof(unsigned char) * w);
		memset(field[i], 32 , w);
	}

	return field;
}

/**
 * spot array access procedure. Mark text output spots
 * @author Hj. Malthaner (hansjoerg.malthaner@gmx.de)
 */
static void set_spots(const int x, const int y, const int n, const bool v)
{
	int i;

	for (i = x; i < x + n; i++)
	{
		spots[i][y] = v;
	}
}


/***********************************************/
#endif /* USE_ISO */

/***********************************************/

/*** Function hooks needed by "Term" ***/

static void Term_init_sdl(term *t)
{
	term_data *td = (term_data*)(t->data);
	DB("Term_init_sdl");
	/* XXX XXX XXX */
}

static void Term_nuke_sdl(term *t)
{
	term_data *td = (term_data*)(t->data);
	DB("Term_nuke_sdl");
	/* XXX XXX XXX */
}

static errr Term_user_sdl(int n)
{
	term_data *td = (term_data*)(Term->data);
	DB("Term_user_sdl");
	/* XXX XXX XXX */

	/* Unknown */
	return (1);
}

/* KEYPRESS_STRING repeatedly sends characters to the terminal
XXX - should implement routine from maim-sdl.c, it's sooo much
cleaner */
#define KEYPRESS_STRING(str) \
strcpy(buf,str); \
n = buf; \
while (*n != '\0') { \
	Term_keypress((int)(*(n++))); \
}

/* function prototype */
void manipulationMode(void);
void redrawAllTerminals(void);
/* This is the main event handling routine that will be called
whenever an event is pulled off of the queue (in Term_xtra_sdl())*/
void handleEvent(SDL_Event *event)
{
	static char buf[24];	/* a buffer used when passing key names */
	char *n;				/* and a pointer to manipulate this buffer */

	switch( event->type )
	{
	case SDL_KEYDOWN:
		{
			/* handle key presses */

			/* I'm reading that as long as the upper 9 bits of the unicode
			 * value are zero, then the lower 7 bits are direct ASCII characters.
			 * Furthermore, it seems that all basic keys return non-zero values
			 * for the lower 7 bits, but function keys and other various things
			 * return 0000000 for the lower 7 bits.
			 * Basically, if the lower 7 bits are zero, do something special
			 *  (like start a macro), but otherwise just pass along the ASCII
			 * code!
			 */
			byte ascii_part = event->key.keysym.unicode & 0x00ff;

			/* gimme the key name */
			printf("Key is: %s\n",SDL_GetKeyName(event->key.keysym.sym));

			/* allow for full screen toggling! */
			if ((event->key.keysym.sym == SDLK_RETURN) && \
				(SDL_GetModState() & KMOD_ALT))
			{
				SDL_WM_ToggleFullScreen(screen);
				/* toggle the internal full screen flag */
				arg_full_screen = (arg_full_screen ? FALSE : TRUE);
			}

			/* entry into window manipulation mode */
			if ((event->key.keysym.sym == SDLK_RETURN) && \
				(SDL_GetModState() & KMOD_CTRL))
			{
				DB("Manipulation mode!");
				manipulationMode();
			}

#ifdef USE_ISO
			/* toggle tile size */
			if (event->key.keysym.sym == SDLK_SCROLLOCK) 
			{
			    switch (display_get_tile_size())
			    {
				case 32:
				    display_select_tile_size(0);
				    break;
				case 64:
				    display_select_tile_size(1);
				    break;
				default:
				    display_select_tile_size(0);
				    break;
			    }
			    reset_visuals();
			    strcpy(buf, "graf-iso.prf");
			    process_pref_file(buf);
			    refresh_display();
			    SDL_UpdateRect(screen, 0, 0, data[0].size_w, data[0].size_h);
			}
			
			/* cycle grid type none/objects+monsters only/full */
			if ((event->key.keysym.sym == '#') && \
				(SDL_GetModState() & KMOD_ALT))
			{
			    set_grid(get_grid()+1);
			    refresh_display();
			}

#endif
			
			/*printf("ascii_part: %d\n",ascii_part);*/
			if (ascii_part)
			{
				/* We have now determined that the ASCII part is not '0', so
				 we can safely pass along the ASCII value! */
				Term_keypress(ascii_part);
			}
			else
			{
				/* We want to ignore keypresses that are simply the modifier
				keys*/
				if (!( (event->key.keysym.sym == SDLK_RSHIFT) |
					(event->key.keysym.sym == SDLK_LSHIFT) |
					(event->key.keysym.sym == SDLK_RALT) |
					(event->key.keysym.sym == SDLK_LALT) |
					(event->key.keysym.sym == SDLK_RCTRL) |
					(event->key.keysym.sym == SDLK_LCTRL) ))
				{

					/* now build a macro string using the modifiers together
					with the key that was just pressed*/

					/* As for the formatting...
					 * We pass the key press and modifiers as follows:
					 * \[ctrl-alt-shift-"key name"]
					 * following the previously established convention...
					 *
					 * All of the things that happen are defined in pref-sdl.prf
					 */
					
					KEYPRESS_STRING("\[");	/*Output the first part... */
					/* See if a control key is down */
					if (event->key.keysym.mod & KMOD_CTRL)
					{
						KEYPRESS_STRING("ctrl-");
					}
					/* See if an alt key is down */
					if (event->key.keysym.mod & KMOD_ALT)
					{
						KEYPRESS_STRING("alt-");
					}
					/* See if a shift key is down */
					if (event->key.keysym.mod & KMOD_SHIFT)
					{
						KEYPRESS_STRING("shift-");
					}

					/* Add in the name of whatever key was pressed */
					KEYPRESS_STRING(SDL_GetKeyName(event->key.keysym.sym));

					/* and end it... */
					KEYPRESS_STRING("]");
				}
			}
			break;
		}
	case SDL_QUIT:
		{
			/* handle quit requests */
			DB("Emergency Blit");
			redrawAllTerminals();
			/*sdl_quit("Quitting!\n");*/
			break;
		}
	default:
		{
			break;
		}
	}
}

/* declare the screen clearing function used below */
void eraseTerminal();
void drawTermStuff(term_data *td, SDL_Rect *rect);
static errr Term_xtra_sdl(int n, int v)
{
	static SDL_Event event;
	term_data *td;


	/* Analyze */
	switch (n)
	{
	case TERM_XTRA_EVENT:
		{
			if (v)
			{
				/* Perform event checking with blocking */
				SDL_WaitEvent( &event );
				handleEvent( &event );
			} else {
				/* Perform event checking without blocking */
				if (SDL_PollEvent(&event)){
					/* We found an event! */
					handleEvent(&event);
				}
			}
			return(0);
		}

	case TERM_XTRA_FLUSH:
		{
			/* Keep doing events until the queue is empty! */
			while (SDL_PollEvent(&event))
			{
				handleEvent(&event);
			}
			return (0);
		}

	case TERM_XTRA_CLEAR:
		{
			/* Clear the terminal */
			DB("TERM_XTRA_CLEAR");
			suspendUpdate = TRUE;
			eraseTerminal();
			return (0);
		}

	case TERM_XTRA_SHAPE:
		{
			/*
			 * Set the cursor visibility XXX XXX XXX
			 *
			 * This action should change the visibility of the cursor,
			 * if possible, to the requested value (0=off, 1=on)
			 *
			 * This action is optional, but can improve both the
			 * efficiency (and attractiveness) of the program.
			 */

			return (0);
		}

	case TERM_XTRA_FROSH:
		{
			/*
			 * Flush a row of output XXX XXX XXX
			 *
			 * This action should make sure that row "v" of the "output"
			 * to the window will actually appear on the window.
			 *
			 * This action is optional, assuming that "Term_text_xxx()"
			 * (and similar functions) draw directly to the screen, or
			 * that the "TERM_XTRA_FRESH" entry below takes care of any
			 * necessary flushing issues.
			 */

			return (1);
		}

	case TERM_XTRA_FRESH:
		{
			/*
			 * Flush output XXX XXX XXX
			 *
			 * This action should make sure that all "output" to the
			 * window will actually appear on the window.
			 *
			 * This action is optional, assuming that "Term_text_xxx()"
			 * (and similar functions) draw directly to the screen, or
			 * that the "TERM_XTRA_FROSH" entry above takes care of any
			 * necessary flushing issues.
			 */

#ifdef USE_ISO
	 		// Hajo:
			// refresh the graphical view

			refresh_display();
			SDL_UpdateRect(screen, 0, 0, data[0].size_w, data[0].size_h);
//			SDL_UpdateRect(td->face, 0, 0, 80*td->w, 24*td->h);
#else /* regular SDL */
			/* If terminal display has been held for any reason,
			then update the whole thing now!*/			
			DB("TERM_XTRA_FRESH");
			if (suspendUpdate)
			{
				DB("  update WAS suspended... updating now");
				td = (term_data*)(Term->data);
				suspendUpdate = FALSE;				
				drawTermStuff(td,NULL);
			}
#endif /* USE_ISO */			
			return (0);
		}

	case TERM_XTRA_NOISE:
		{
			/*
			 * Make a noise XXX XXX XXX
			 *
			 * This action should produce a "beep" noise.
			 *
			 * This action is optional, but convenient.
			 */

			return (1);
		}

	case TERM_XTRA_SOUND:
		{
			/*
			 * Make a sound XXX XXX XXX
			 *
			 * This action should produce sound number "v", where the
			 * "name" of that sound is "sound_names[v]".  This method
			 * is still under construction.
			 *
			 * This action is optional, and not very important.
			 */

			return (1);
		}

	case TERM_XTRA_BORED:
		{
			/* Perform event checking without blocking */
			if (SDL_PollEvent(&event)){
				/* We found an event! */
				handleEvent(&event);
			}
			return(0);
		}

	case TERM_XTRA_REACT:
		{
			/*
			 * React to global changes XXX XXX XXX
			 *
			 * For example, this action can be used to react to
			 * changes in the global "color_table[256][4]" array.
			 *
			 * This action is optional, but can be very useful for
			 * handling "color changes" and the "arg_sound" and/or
			 * "arg_graphics" options.
			 */
#ifdef USE_ISO
	    	strcpy(buf, "graf-iso.prf");
			process_pref_file(buf);
#endif /* USE_ISO */
			return (1);
		}

	case TERM_XTRA_ALIVE:
		{
			/*
			 * Change the "hard" level XXX XXX XXX
			 *
			 * This action is used if the program changes "aliveness"
			 * by being either "suspended" (v=0) or "resumed" (v=1)
			 * This action is optional, unless the computer uses the
			 * same "physical screen" for multiple programs, in which
			 * case this action should clean up to let other programs
			 * use the screen, or resume from such a cleaned up state.
			 *
			 * This action is currently only used by "main-gcu.c",
			 * on UNIX machines, to allow proper "suspending".
			 */

			return (1);
		}

	case TERM_XTRA_LEVEL:
		{
			/*
			 * Change the "soft" level XXX XXX XXX
			 *
			 * This action is used when the term window changes "activation"
			 * either by becoming "inactive" (v=0) or "active" (v=1)
			 *
			 * This action can be used to do things like activate the proper
			 * font / drawing mode for the newly active term window.  This
			 * action should NOT change which window has the "focus", which
			 * window is "raised", or anything like that.
			 *
			 * This action is optional if all the other things which depend
			 * on what term is active handle activation themself, or if only
			 * one "term_data" structure is supported by this file.
			 */

			return (1);
		}

	case TERM_XTRA_DELAY:
		{
			/*
			 * Delay for some milliseconds XXX XXX XXX
			 *
			 * This action is useful for proper "timing" of certain
			 * visual effects, such as breath attacks.
			 *
			 * This action is optional, but may be required by this file,
			 * especially if special "macro sequences" must be supported.
			 */

			/* I think that this command is system independent... */
			/*sleep(v/1000);*/
			/* main-x11 uses usleep(1000*v); */
			/* main-win uses Sleep(v); */
			return (1);
		}

	case TERM_XTRA_GET_DELAY:
		{
			/*
			 * Get Delay of some milliseconds XXX XXX XXX
			 * place the result in Term_xtra_long
			 *
			 * This action is useful for proper "timing" of certain
			 * visual effects, such as recording cmovies.
			 *
			 * This action is optional, but cmovies wont perform
			 * good without it
			 */

			return (1);
		}
	}

	/* Unknown or Unhandled action */
	return (1);
}

/*************************************************
 GRAPHICS ROUTINES
 *************************************************/

/* wrapper routine for creating an RGB surface with given height and
 width that corresponds to desired color depth and respects the system
 pixel format */
SDL_Surface *createSurface(int width, int height)
{
	SDL_Surface *surf;
	int surface_type;
	
	if (videoInfo->hw_available)
		surface_type = SDL_HWSURFACE;
	else
		surface_type = SDL_SWSURFACE;
	
	/* XXX need to make RGBA masks correspond to system pixel format! */
	switch (arg_bpp)
	{
		case 8:
		{
			/* I really don't know if 8 bpp is even possible, but here it is */
			surf = SDL_CreateRGBSurface(surface_type,width,\
				height,8,0xc0,0x30,0x0c,0x03);
			break;
		}
		case 16:
		{
			surf = SDL_CreateRGBSurface(surface_type,width,\
				height,16,0xf000,0x0f00,0x00f0,0x000f);
			break;
		}
		case 24:
		{
			surf = SDL_CreateRGBSurface(surface_type,width,\
				height,24,0xfc0000,0x03f000,0x000fc0,0x000030);
			break;
		}
		case 32:
		{
			surf = SDL_CreateRGBSurface(surface_type,width,\
				height,32,0xff000000,0x00ff0000,0x0000ff00,0x000000ff);
			break;
		}
		default:
		{
			surf = NULL;
			break;
		}
	}
	
	if (surf == NULL)
		sdl_quit("Bad Surface Creation!");
	
	return surf;
}

/* Take a rectangle in terminal coordinates and then transform it into
screen coordinates; td is the term_data that this rect belongs to */
void term_to_screen(SDL_Rect *termrect, term_data *td)
{
	termrect->x += td->rect.x;
	termrect->y += td->rect.y;
}

/* Do the opposite, take a rectangle in screen coordinates and transform 
it into the terminal coordinates of the given term_data */
void screen_to_term(SDL_Rect *scrrect, term_data *td)
{
	scrrect->x -= td->rect.x;
	scrrect->y -= td->rect.y;
}

/* A macro that determines if the 'top' rectangle completely occludes the
'bottom' rectangle */
#define BLOCKS(top,bottom) \
( (top.x <= bottom.x) & ((top.x+top.w)>=(bottom.x+bottom.w)) & \
 (top.y <= bottom.y) & ((top.y+top.h)>=(bottom.y+bottom.h)) )

#define INTERSECT(r1,r2) \
!( ( (r1.x > (r2.x+r2.w)) | (r2.x > (r1.x+r1.w)) ) & \
  ( (r1.y > (r2.y+r2.h)) | (r2.y > (r1.y+r1.h)) ) )

/* A function to calculate the intersection of two rectangles. Takes base
rectangle and then updates it to include only the rectangles that intersect 
with the test rectangle. If there is an intersection, the function returns
TRUE and base now contains the intersecting rectangle. If there is no
intersection, then the function returns FALSE */
bool intersectRects(SDL_Rect *base, SDL_Rect *test)
{
	if (INTERSECT((*base),(*test)))
	{
		/* Scoot the x-coordinates for the left side*/
		if ( test->x > base->x )
		{
			base->w -= test->x - base->x;
			base->x = test->x;
		}
		/* Scoot the x-coordinates for the right side*/		
		if ( (test->x + test->w) < (base->x + base->w) )
		{
			base->w = test->x + test->w - base->x;
		}
		/* Scoot the upper y-coordinates */
		if ( test->y > base->y )
		{
			base->h -= test->y - base->y;
			base->y = test->y;
		}
		/* Scoot the lower y-coordinates */		
		if ( (test->y + test->h) < (base->y + base->h) )
		{
			base->h = test->y + test->h - base->y;
		}
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

/* A function to calculate the join of two rectangles; the first argument is
changed to the joined rectangle */
SDL_Rect joinRects(SDL_Rect *r1, SDL_Rect *r2)
{
	SDL_Rect out = {0,0,0,0};
	
	if ( (r1 != NULL) & (r2 != NULL) )
	{
		/* Lower x-coordinate */
		if ( r2->x < r1->x )
			out.x = r2->x;
		else
			out.x = r1->x;
		/* Upper x-coordinate */
		if ( (r2->x+r2->w) > (r1->x+r1->w) )
			out.w = (r2->x+r2->w) - out.x;
		else
			out.w = (r1->x+r1->w) - out.x;
		/* Lower y-coordinate */
		if ( r2->y < r1->y )
			out.y = r2->y;
		else
			out.y = r1->y;
		if ( (r2->y+r2->h) > (r1->y+r1->h) )
			out.h = (r2->y+r2->h) - out.y;
		else
			out.h = (r1->y+r1->h) - out.y;
	}
	return out;
}
			
/* Given a term_data (and its associated screen) and a rectangle in terminal
coordinates (with NULL signifying to take the whole terminal surface), blit
graphics from the term_data surface to the screen, using the term_data's rect
to indicate how terminal coordinates transform into screen coordinates.
This is complicated, however, by the possibility that the indicated area may be
occluded by overlaying terminals. In this case, if the target area is
completely occluded, nothing will be done. If partially occluded, it will be
drawn, but occluding terminals will then re-blit to re-cover the area. */
void drawTermStuff(term_data *td, SDL_Rect *rect)
{
	int n = 0, i;
	bool block = FALSE, cover = FALSE;
	SDL_Rect spot, isect_term, isect_scr;

	/* first of all, if updating is suspended, do nothing! */
	if (!suspendUpdate)
	{
		/* find out which number in the ordered stack of screens that this
		terminal is */
		while ((term_order[n] != td) & (n < MAX_CONSOLE_COUNT))
		{
			n++;
		}
		if (n == MAX_CONSOLE_COUNT)
			printf("Could not find terminal in display list...\n");
		/* now loop through and see if any terminals completely occlude 
		the desired spot; if num=0, note that this will be skipped */
		if (rect == NULL)
		{
			/* Grab the whole terminal screen */
			spot.x = 0;	spot.y = 0;
			spot.w = td->surf->w; spot.h = td->surf->h;
		}
		else
		{
			/* Just copy the given area */
			spot.x = rect->x; spot.y = rect->y;
			spot.w = rect->w; spot.h = rect->h;
		}
		term_to_screen(&spot,td);
		i = n;
		while (i--)
		{
			if (BLOCKS(term_order[i]->rect,spot))
			{
				/* Higher terminal completely occludes this spot */
				block = TRUE;
				DB("  Blocks!");
			}
			else if (INTERSECT(term_order[i]->rect,spot))
			{
				/* Partial occlusion */
				cover = TRUE;
				DB("  Covers!");
			}
		}
		/* If any of the higher terminals blocked, then don't do
		anything */
		if (!block)
		{
			/*printf("Blitting to %d %d %d %d\n",spot.x,spot.y,spot.w,spot.h);*/
			/* First of all, draw the graphics */
			SDL_BlitSurface(td->surf,rect,screen,&spot);
			if (cover)
			{
				printf("covering...");
				/* There are covering terminals, so go through and blit all
				partially occluding ones */
				while (n--)
				{
					/* copy spot to find the intersect */
					isect_scr.x = spot.x; isect_scr.y = spot.y;
					isect_scr.w = spot.w; isect_scr.h = spot.h;
					if (intersectRects(&isect_scr,&(term_order[n]->rect)))
					{
						/* this terminal intersects... re-blit */
						/* first, convert to term coordinates */
						isect_term.x = isect_scr.x; isect_term.y = isect_scr.y;
						isect_term.w = isect_scr.w; isect_term.h = isect_scr.h;						
						screen_to_term(&isect_term,term_order[n]);
						/* blit from term coordinates to screen coordinates */
						SDL_BlitSurface(term_order[n]->surf,&isect_term,\
							screen,&isect_scr);
					}
				}
			}
			/* Now update what was drawn */
			DB("Update");
			SDL_UpdateRects(screen,1,&spot);
		}
	}
}
		
/* utility routine for creating and setting the color of the cursor;
it could be useful for setting a new cursor color if desired.
Could later be expanded to do other stuff with the cursor,
like a hollow rectangle a la main-win.c or even a graphic */
void createCursor(byte r, byte g, byte b, byte a)
{
	/* free the cursor if it exists */
	if (cursor != NULL)
		SDL_FreeSurface(cursor);

	/* and create it anew! (or the first time) */
	cursor = createSurface(t_width,t_height);
	
	/* be sure to use alpha channel when blitting! */
	SDL_SetAlpha(cursor,SDL_SRCALPHA,0);
	
	/* just set the color for now - drawing rectangles
	needs surface locking for some setups */
	cursor_color = SDL_MapRGBA(cursor->format,r,g,b,a);
	SDL_LOCK(cursor);
	SDL_FillRect(cursor,NULL,cursor_color);
	SDL_UNLOCK(cursor);
}

/* Cursor Display routine - just blits the global cursor
surface onto the correct location */
static errr Term_curs_sdl(int x, int y)
{
#ifdef USE_ISO
	highlite_spot(x, y);
#else /* regular SDL */
	term_data *td = (term_data*)(Term->data);
	static SDL_Rect base;

	/* calculate the position to place the cursor */
	base.x = td->surf->clip_rect.x + x*t_width;
	base.y = td->surf->clip_rect.y + y*t_height;
	base.w = t_width;
	base.h = t_height;
	
	/* blit the cursor over top of the given spot;
	note that surface should not be locked
	(see note in Term_text_sdl() below) */
	SDL_BlitSurface(cursor,NULL,td->surf,&base);

	/* Now draw to the main screen */
	drawTermStuff(td,&base);
#endif	/* USE_ISO */
	/* Success */
	return (0);
}

/* routine for wiping terminal locations - simply draws
a black rectangle over the offending spots! */
static errr Term_wipe_sdl(int x, int y, int n)
{
	static SDL_Rect base;
	term_data *td = (term_data*)(Term->data);

	/* calculate boundaries of the area to clear */
	base.x = td->surf->clip_rect.x + x*t_width;
	base.y = td->surf->clip_rect.y + y*t_height;
	base.w = n*t_width;
	base.h = t_height;

	SDL_LOCK(td->surf);
	
	/* blank the screen area */
	SDL_FillRect(td->surf, &base, td->black);

	SDL_UNLOCK(td->surf);

	/* And... UPDATE the rectangle we just wrote to! */
	drawTermStuff(td,&base);

	/* Success */
	return (0);
}
	
/* Perform a full clear of active terminal; redraw the borders.*/
void eraseTerminal(void)
{
	static SDL_Rect base;
	term_data *td = (term_data*)(Term->data);

	/* temporarily remove clipping rectangle */
	SDL_SetClipRect(td->surf,NULL);

	SDL_LOCK(td->surf);	
	/* flood terminal with border color */
	SDL_FillRect(td->surf,NULL,td->border_color);
	
	/* get smaller rectangle to hollow out window */
	base.x = td->border_thick;
	base.y = td->border_thick;
	base.w = td->rect.w - 2*td->border_thick;
	base.h = td->rect.h - 2*td->border_thick;
	
	/* hollow out terminal */
	SDL_FillRect(td->surf,&base,td->black);

	SDL_UNLOCK(screen);
	
	/* reset clipping rectangle */
	base.x += td->cushion_x_top;
	base.y += td->cushion_y_top;
	base.w -= td->cushion_x_top + td->cushion_x_bot;
	base.h -= td->cushion_y_top + td->cushion_y_bot;
	SDL_SetClipRect(td->surf,&base);
	printf("Clip rect: %d %d %d %d\n",base.x,base.y,base.w,base.h);
	/* And... UPDATE the whole thing */
	drawTermStuff(td,NULL);
	
}

/*
 * Draw some text on the screen
 *
 * This function should actually display an array of characters
 * starting at the given location, using the given "attribute",
 * and using the given string of characters, which contains
 * exactly "n" characters and which is NOT null-terminated.
 *
 * You may assume "valid" input if the window is properly sized.
 *
 * You must be sure that the string, when written, erases anything
 * (including any visual cursor) that used to be where the text is
 * drawn.  On many machines this happens automatically, on others,
 * you must first call "Term_wipe_xxx()" to clear the area.
 *
 * In color environments, you should activate the color contained
 * in "color_data[a & 0x0F]", if needed, before drawing anything.
 *
 * You may ignore the "attribute" if you are only supporting a
 * monochrome environment, since this routine is normally never
 * called to display "black" (invisible) text, including the
 * default "spaces", and all other colors should be drawn in
 * the "normal" color in a monochrome environment.
 *
 * Note that if you have changed the "attr_blank" to something
 * which is not black, then this function must be able to draw
 * the resulting "blank" correctly.
 *
 * Note that this function must correctly handle "black" text if
 * the "always_text" flag is set, if this flag is not set, all the
 * "black" text will be handled by the "Term_wipe_xxx()" hook.
 */
static errr Term_text_sdl(int x, int y, int n, byte a, const char *cp)
{
#ifdef USE_ISO
	if (a < 16)
	{
		set_spots(x, y, n, TRUE);
	}
	else
	{
		set_spots(x, y, n, FALSE);
	}
#else
	term_data *td = (term_data*)(Term->data);
	static SDL_Rect base;
	SDL_Rect base_back;
	int i = n;
	char old = 0;

	/* calculate place to clear off and draw to */
	base.x = td->surf->clip_rect.x + x*td->tile_width;
	base.y = td->surf->clip_rect.y + y*td->tile_height;
	base.w = n*td->tile_width;
	base.h = td->tile_height;

	base_back = base;

	SDL_LOCK(screen);

	/* blank the drawing area */
	SDL_FillRect(td->surf, &base, td->black);

	SDL_UNLOCK(screen);

	/* Note that SDL docs specify that SDL_BlitSurface should not be called
	 on locked surfaces... since the character printing routine below revolves
	 around blitting, the surface has been unlocked first*/

	/* loop through the input string, drawing characters */
	i = n;
	old = 0;
	while (i--)
	{
		/* Output the character... */
		/* If character has not changed, then just blit the old surface into
		 the new location to save effort*/
		if (*cp == old)
		{
			/* the desired character/color combo is already on the work surf */
			/* just blit it! */
			SDL_BlitSurface(worksurf,NULL,td->surf,&base);
		} else {
			/* copy the desired character onto working surface */
			SDL_BlitSurface(text[*cp],NULL,worksurf,NULL);
			/* color our crayon surface with the desired color */
			SDL_FillRect(crayon,NULL,color_data[a&0x0f]);
			/* apply the color to the character on the working surface */
			SDL_BlitSurface(crayon,NULL,worksurf,NULL);
			/* and blit it onto our screen! */
			SDL_BlitSurface(worksurf,NULL,td->surf,&base);
		}
		/* Move to the next position */
		base.x += t_width;
		/* Store the old character */
		old = *cp;
		/* Increment the character pointer */
		cp++;
	}

	/* And update */
	drawTermStuff(td,&base_back);

#endif /* USE_ISO */	
	/* Success */
	return (0);
}

/*
 * Draw some attr/char pairs on the screen
 *
 * This routine should display the given "n" attr/char pairs at
 * the given location (x,y).  This function is only used if one
 * of the flags "always_pict" or "higher_pict" is defined.
 *
 * You must be sure that the attr/char pairs, when displayed, will
 * erase anything (including any visual cursor) that used to be at
 * the given location.  On many machines this is automatic, but on
 * others, you must first call "Term_wipe_xxx(x, y, 1)".
 *
 * With the "higher_pict" flag, this function can be used to allow
 * the display of "pseudo-graphic" pictures, for example, by using
 * the attr/char pair as an encoded index into a pixmap of special
 * "pictures".
 *
 * With the "always_pict" flag, this function can be used to force
 * every attr/char pair to be drawn by this function, which can be
 * very useful if this file can optimize its own display calls.
 *
 * This function is often associated with the "arg_graphics" flag.
 *
 * This function is only used if one of the "higher_pict" and/or
 * "always_pict" flags are set.
 */
#ifndef USE_ISO
static errr Term_pict_sdl(int x, int y, int n, const byte *ap, const char *cp)
{
	term_data *td = (term_data*)(Term->data);
	DB("Term_pict_sdl");
	/* XXX XXX XXX */
	
#else
// for ISO-view we need USE_TRANSPARENCY and USE_EGO_GRAPHICS defined
static errr Term_pict_sdl(int x, int y, int n, const byte *ap, const char *cp, const byte *tap, const char *tcp, const byte *eap, const char *ecp)
{
	/* Hajo: memorize output */
	memcpy(&iso_ap[y][x], ap, n);
	memcpy(&iso_cp[y][x], cp, n);
	memcpy(&iso_atp[y][x], tap, n);
	memcpy(&iso_ctp[y][x], tcp, n);
	memcpy(&iso_aep[y][x], eap, n);
	memcpy(&iso_cep[y][x], ecp, n);

	// here is no text
	set_spots(x, y, n, FALSE);

#endif /* USE_ISO */
	/* Success */
	return (0);
}

/*************************************************
 SPECIAL TERMINAL WINDOW MANIPULATION ROUTINES
 *************************************************/

/* macro for bounding a value between two given values */
#define BOUND(val,low,high) \
if (val < low) \
{ \
	val = low; \
} \
else if (val > high) \
{ \
	val = high; \
}

/* two macros to get the adjusted maximums for window
positions... eg the screen width minus the width of the
window is the maximum x-position that the window can
be set at. */
#define MAX_X(td) \
( arg_width - td->rect.w )

#define MAX_Y(td) \
( arg_height - td->rect.h )

/* another two macros that give maximum window widths
based on screen size and current window position together
width tile widths/heights */
#define MAX_WIDTH(td) \
( (int)floorf((arg_width - td->rect.x - 2*td->border_thick - \
	td->cushion_x_bot - td->cushion_x_top )/td->tile_width))

#define MAX_HEIGHT(td) \
( (int)floorf((arg_height - td->rect.y - 2*td->border_thick - \
	td->cushion_y_bot - td->cushion_y_top )/td->tile_height))

/* update the width and height of given term's rectangle by simply
multiplying the tile count by tile size and adding in cushions and
borders */
#define UPDATE_SIZE(td) \
	td->rect.w = (td->cols)*td->tile_width + td->cushion_x_top \
		+ td->cushion_x_bot + 2*td->border_thick; \
	td->rect.h = (td->rows)*td->tile_height + td->cushion_y_top \
		+ td->cushion_y_bot + 2*td->border_thick

void recompose(void);

/* Resize the active terminal with new width and height.
Note that his involves a complicated sequence of events...
Details to follow below! */
void resizeTerminal(int width, int height)
{
	term_data *td = (term_data*)(Term->data);
	
	/* First of all, bound the input width and height to satisfy
	these conditions:
	- The main ToME window should be at least 80 cols, 24 rows
	- no part of each window should be drawn off screen....
	  I'm including borders in this restriction!
	But no bounds checking needs to take place if the input width
	and height are unchanged....
	*/

	if (td == &data[0])
	{
		/* The active terminal is the main ToME window...
		don't let the width get below 80, don't let the heights below
		24, and don't let it leak off of the edge! */
		if (width != td->cols)
		{
			BOUND(width,80,MAX_WIDTH(td));
		}
		if (height != td->rows)
		{
			BOUND(height,24,MAX_HEIGHT(td));
		}
	}
	else
	{
		/* This is not the main window... just make sure it
		doesn't shrink to nothing or go past the edge */
		if (width != td->cols)
		{
			BOUND(width,1,MAX_WIDTH(td));
		}
		if (height != td->rows)
		{
			BOUND(height,1,MAX_HEIGHT(td));
		}
	}
	
	/* Okay, now make sure that something has ACTUALLY changed 
	before doing anything */
	if ((width != td->cols) || (height != td->rows))
	{
		
		/* Now, ask zterm to please resize the term structure! */
		Term_resize(width,height);

		/* Reactivate, since Term_resize seems to activate the
		main window again...*/
		Term_activate(&td->t);
		
		/* It might not have resized completely to the new
		size we wanted (some windows have size limits it seems,
		like the message window). So, update our structure with
		the size that were actually obtained.*/
		td->cols = Term->wid;
		td->rows = Term->hgt;
		
		/* And recalculate the sizes */
		UPDATE_SIZE(td);		
		
		/* Create a new surface that can hold the updated size */
		SDL_FreeSurface(td->surf);
		td->surf = createSurface(td->rect.w,td->rect.h);

		/* Now we should be in business for a complete redraw! */
		Term_redraw();
		
		/* Re-blit everything so it looks good */
		recompose();
		
		/* That's it! */
	}
}

/* Move the terminal around... a much simpler action that involves
just changing the pos_x/pos_y values and redrawing!*/
void moveTerminal(int x, int y)
{
	term_data *td = (term_data*)(Term->data);
	
	/* Now, the window is being shifted about... much simpler
	situation to handle! But of course, the window must not
	drift too far or else parts will be hanging off the screen
	and may lead to errors - bound the input positions to
	prevent this unfortunate situation... do nothing if the
	input is no different than the current */
	if (x != td->rect.x)
	{
		BOUND(x,0,MAX_X(td));
	}
	if (y != td->rect.y)
	{
		BOUND(y,0,MAX_Y(td));
	}
	
	/* Okay, now make sure that something changed before doing
	anything */
	if ((x != td->rect.x) || (y != td->rect.y))
	{
		/* Now update OUR structure */
		td->rect.x = x;
		td->rect.y = y;
		
		/* Then do a reblit to see the results */
		recompose();
		
		/* That's it! */
	}
}

/* Routine to bring a given term_data to the top of the drawing stack */
void bringToTop(int current)
{
	term_data *td;
	term_data *tc;
	int n = 0;
	int i;
	
	/* Get the pointer to the desired term_data from the data structure */
	td = &data[current];
	
	printf("Current stack: \n");
	for (i=0;i<arg_console_count;i++)
	{
		printf("  %d: %p\n",i,term_order[i]);
	}
	printf("\n");
	
	/* Find the number in the term_order stack */
	while ((term_order[n] != td) & (n < MAX_CONSOLE_COUNT))
	{
		n++;
	}
	if (n == MAX_CONSOLE_COUNT)
		printf("Could not find terminal in display list...\n");
	
	printf("Order is %d\n",n);
	
	/* Now move all lower-indexed pointers up one index */
	while (n)
	{
		printf(" move %d to %d\n",n-1,n);
		printf("   %p\n",term_order[n-1]);
		printf("      %p\n",term_order[n]);
		term_order[n] = term_order[n-1];
		n--;
	}
	/* And stick this term_data pointer on top */
	term_order[0] = td;
	
	printf("Final stack: \n");
	for (i=0;i<arg_console_count;i++)
	{
		printf("  %d: %p\n",i,term_order[i]);
	}
	printf("\n");
	
}

/* This utility routine will cycle the active term to the
next available in the data[] array. It will then do a
redraw of this term so that it is ready to be manipulated.
The input is the current active terminal, and the output is
the new active terminal */
int cycleTerminal(int current)
{
	/* redraw the current term to get rid of its purple
	border */
	data[current].border_color = data[current].white;
	Term_redraw();
	
	/* increment the terminal number*/
	current++;
	/* now do a little modulo cycle action and 
	activate the next term! */
	current %= arg_console_count;
	Term_activate(&(data[current].t));
	
	/* before redrawing, set the border color to purple to
	indicate that this terminal is being manipulated*/
	data[current].border_color = data[current].purple;

	/* then bring this terminal to the top of the order, so it is drawn on
	top during manipulation mode */
	bringToTop(current);
	
	/* and do a complete redraw */
	Term_redraw();
	
	/* return the current terminal... */
	return current;
}

/* This routine will simply re-blit all of the surfaces onto the main screen,
respecting the current term_order */
void recompose(void)
{
	int i = arg_console_count;
	/* do a complete screen wipe */
	SDL_LOCK(screen);
	SDL_FillRect(screen,NULL,screen_black);
	SDL_UNLOCK(screen);
	
	/* cycle through the term_order */
	while (i--)
	{
		SDL_BlitSurface(term_order[i]->surf,NULL,screen,&(term_order[i]->rect));
	}
	
	/* Update everything */
	SDL_REDRAW_SCREEN;
}

/* This utility routine will completely blank the screen and
then cycle through all terminals, performing a Term_redraw()
on each and every term that is being used (according to
arg_term_count that is). The terminals will be redrawn
last-to-first, so that the main is over top of everything */
void redrawAllTerminals(void)
{
	int i = arg_console_count;
	DB("Total redraw");
	/* do a complete screen wipe */
	SDL_LOCK(screen);
	SDL_FillRect(screen,NULL,screen_black);
	SDL_UNLOCK(screen);

	while (i--)
	{
		/* Re-order the terminals */
		term_order[i] = &data[i];
	}
	
	i = arg_console_count;
	/* cycle down through each terminal */
	while (i--)
	{
		/* Activate this terminal */
		Term_activate(&(data[i].t));
		
		/* Make its border white since manipulation mode is over */
		data[i].border_color = data[i].white;

		/* And redraw it */
		Term_redraw();
	}
	/* Loop will end on i=0 ! */

	printf("Current stack: \n");
	for (i=0;i<arg_console_count;i++)
	{
		printf("  %d: %p\n",i,term_order[i]);
	}
	printf("\n");
	
	/* now update the screen completely, just in case*/
	SDL_REDRAW_SCREEN;
}

/* This is the special event handling function for doing
terminal window manipulation! When special manipulation
mode is activated, execution goes here. This special mode
has its own keypresses. To begin with, the main terminal
border is highlighted (in purple) to indicate that it is
being manipulated. The following keypresses are accepted:
-Space: switches between editing modes. The system begins
  in position editing mode, and Enter will toggle size
  (row/col) editing mode.
-Arrows: increments/decrements the position/size in an
  intuitive way! ;) Some modifiers are accepted in order
  to speed things up on very high resolution screens:
   . with shift down, increment is five
   . with ctrl down, increment is ten
   . with both, increment is fifty!
  Of course, no movement or resize can cause the window
  to leave the confines of the screen, so using the big
  jump is safe.
-Enter: cycles to the next available terminal to edit.
-Escape: quits manipulation mode, performing one final
  redraw to take into account all changes.
*/
void manipulationMode(void)
{
	term_data *td;
	SDL_Event event;
	bool done = FALSE, moveMode = TRUE;
	int mouse_x, mouse_y;
	int value = 0, delta_x = 0, delta_y = 0;
	int current_term;
	SDL_Surface backup;
	
	/* Begin by redrawing the main terminal with its
	purple border to signify that it is being edited*/

	/* start with the main terminal */
	current_term = 0;
	
	/* get the pointer */
	td = &data[0];
	
	/* before redrawing, set the border color to purple to
	indicate that this terminal is being manipulated*/
	td->border_color = td->purple;
	
	/* and do a complete redraw */
	DB("Term_redraw");
	Term_redraw();
	
	/* Now keep looping until Esc has been pressed. */
	while (!done)
	{
		/* Get the keypress event */
		SDL_WaitEvent(&event);
		/* Make sure that it is a keypress */
		if (event.type == SDL_KEYDOWN)
		{
			/* Act on the keypress! */
			switch (event.key.keysym.sym)
			{
				case SDLK_ESCAPE:
				{
					/* Escape has been pressed, so we're done!*/
					done = TRUE;
					break;
				}
				case SDLK_SPACE:
				{
					/* Space has been pressed: toggle move mode */
					moveMode = ( moveMode ? FALSE : TRUE );
					break;
				}
				case SDLK_RETURN:
				{
					/* Return... cycle the terminals!
					update the current_term appropriately*/
					current_term = cycleTerminal(current_term);
					
					/* Get the new term_data */
					td = &data[current_term];
					
					break;
				}
				case SDLK_RIGHT:
				case SDLK_LEFT:
				case SDLK_DOWN:
				case SDLK_UP:
				{
					/* Increase either the x-position or column
					width - multiply according to modifiers */
					value = 1;
					if (SDL_GetModState() & KMOD_SHIFT)
					{
						/* shift is down... a muliplier of 5 */
						value *= 5;
					}
					if (SDL_GetModState() & KMOD_CTRL)
					{
						/* control is down... multiply by 10 */
						value *= 10;
					}
					
					/* Now, behavior depends on which key was pressed
					and whether we are in moveMode resize mode... */
					
					/* First, set the delta_x/y based on key */
					if (event.key.keysym.sym == SDLK_RIGHT)
					{
						delta_x = 1;
						delta_y = 0;
					}
					if (event.key.keysym.sym == SDLK_LEFT)
					{
						delta_x = -1;
						delta_y = 0;
					}
					if (event.key.keysym.sym == SDLK_DOWN)
					{
						delta_x = 0;
						delta_y = 1;
					}
					if (event.key.keysym.sym == SDLK_UP)
					{
						delta_x = 0;
						delta_y = -1;
					}
					
					/* Now either moveTerminal() or 
					resizeTerminal() based on value of
					moveMode! */
					if (moveMode)
					{
						moveTerminal(td->rect.x + value*delta_x,\
							td->rect.y + value*delta_y);
					}
					else
					{
						resizeTerminal(td->cols + value*delta_x,\
							td->rows + value*delta_y);
					}
					break;
				}
				default:
				{
					break;
				}
			}
		}
		else if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			/* Store the coordinates where the button was pressed */
			mouse_x = event.button.x;
			mouse_y = event.button.y;
		}
		else if (event.type == SDL_MOUSEMOTION)
		{
			/* Mouse is moving... maybe move or resize the window, based
			on the state of the mouse buttons */
			
			/* To keep the motion quick, temporarily ignore all mouse motion
			events until window moving is complete */
			SDL_EventState(SDL_MOUSEMOTION,SDL_IGNORE);
			
			if(event.motion.state & SDL_BUTTON(1))
			{
				/* the left mouse button is down, move the window,
				do a differential based on where the button was pressed */
				moveTerminal(td->rect.x + (event.motion.x - mouse_x), \
					td->rect.y + (event.motion.y - mouse_y));
				/* save the most current mouse location */
				SDL_GetMouseState(&mouse_x,&mouse_y);
			}
			
			if(event.motion.state & SDL_BUTTON(3))
			{
				/* the right mouse button is down, so resize the window;
				do a differential, but divide the number by the tile sizes */
				
				/* see if at least one whole tile width/height has been
				reached */
				int delta_cols, delta_rows;
				delta_cols = (int)floorf(\
					(float)(event.motion.x - mouse_x)/td->tile_width);
				delta_rows = (int)floorf(\
					(float)(event.motion.y - mouse_y)/td->tile_height);
				if ( delta_cols || delta_rows )
				{
					/* something changed, so update */
					resizeTerminal(td->cols + delta_cols, \
						td->rows + delta_rows);
					/* save the most current mouse location */
					SDL_GetMouseState(&mouse_x,&mouse_y);
				}
			}
			
			/* Deal with mouse motion again */
			SDL_EventState(SDL_MOUSEMOTION,SDL_ENABLE);
			
		}
	}
	/* Perform the last redraw to take all changes
	into account */
	redrawAllTerminals();
}

/*************************************************
 INITIALIZATION ROUTINES
 *************************************************/

static errr term_data_init(term_data *td, int i)
{
	term *t = &(td->t);
	char env_var[80];
	cptr val;

	
	/***** load position, size information */
	
	int cols, rows, x, y;	
	
	/* grab the column and row counts from
	environmental variables for now */
	sprintf(env_var,"TOME_NUM_COLS_%d",i);
	val = getenv(env_var);
	/* make sure it is valid */
	if (val != NULL)
	{
		cols = atoi(val);
		/* now make sure that the main window will
		have at least 80x24 */
		if (td == &data[0])
		{
			/* can't really pick an upper bound without
			knowing what the position is... oh well. */
			BOUND(cols,80,255);
		}
	}
	else
	{
		/* no environmental variable... have to guess
		something. If it's the main window, choose
		the minimum. */
		if (td == &data[0])
			cols = 80;
		else
			cols = 5;
	}
	/* do the rows */
	sprintf(env_var,"TOME_NUM_ROWS_%d",i);
	val = getenv(env_var);
	/* make sure it is valid */
	if (val != NULL)
	{
		rows = atoi(val);
		/* now make sure that the main window will
		have at least 80x24 */
		if (td == &data[0])
		{
			/* can't really pick an upper bound without
			knowing what the position is... oh well. */
			BOUND(rows,24,128);
		}
	}
	else
	{
		/* no environmental variable... have to guess
		something. If it's the main window, choose
		the minimum. */
		if (td == &data[0])
			rows = 24;
		else
			rows = 3;
	}
	/* store these values in the term_data structure */
	td->rows = rows;
	td->cols = cols;
	
	/* the position will be loaded from environmental
	variables as well - for the time being*/
	/* x-location */
	sprintf(env_var,"TOME_X_POS_%d",i);
	val = getenv(env_var);
	/* make sure it is valid */
	if (val != NULL)
	{
		x = atoi(val);
		/* now do intelligent position checking */
		BOUND(x,0,MAX_X(td));
	}
	else
	{
		/* no variable, choose something */
		x = 20*i;
	}
	/* y-location */
	sprintf(env_var,"TOME_Y_POS_%d",i);
	val = getenv(env_var);
	/* make sure it is valid */
	if (val != NULL)
	{
		y = atoi(val);
		/* position checking again */
		BOUND(y,0,MAX_Y(td));
	}
	else
	{
		/* no variable */
		y = 20*i;
	}
	/* and store these values into the structure */
	td->rect.x = x;
	td->rect.y = y;
	
	/*********** term structure initializing */
	
	/* Initialize the term 
	 gets: pointer to address, number of columns, number of rows, number
	   of keypresses to queue up (guess 24?)*/
	term_init(t, cols, rows, 24);

	/* Use a "soft" cursor */
	t->soft_cursor = TRUE;

	/* Picture routine flags */
	t->always_pict = FALSE;
	t->higher_pict = FALSE;
	t->always_text = FALSE;

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	/* Hooks */
	t->xtra_hook = Term_xtra_sdl;
	t->curs_hook = Term_curs_sdl;
	t->wipe_hook = Term_wipe_sdl;
	t->text_hook = Term_text_sdl;
#ifdef USE_ISO
	t->pict_hook = Term_pict_sdl;
#endif /* USE_ISO */

	/* Save the data */
	t->data = td;

	/* Activate (important) */
	Term_activate(t);

	/************* finish term_data intializing */
	
	/* name of this term window */
	td->name = angband_term_name[i];

	/* For now, all font is the same size... use global t_width/height */
	td->tile_width = t_width;
	td->tile_height = t_height;

	td->cushion_x_top = 1;
	td->cushion_x_bot = 1;
	td->cushion_y_top = 1;
	td->cushion_y_bot = 1;
	
	/* Now calculate the total width and height*/
	UPDATE_SIZE(td);

	/* Create a surface to draw to */
	td->surf = createSurface(td->rect.w,td->rect.h);
	SDL_SetAlpha(td->surf,0,0);

	/* Key some colors to this surface */
	td->black  = SDL_MapRGB(td->surf->format,  0,  0,  0);
	td->white  = SDL_MapRGB(td->surf->format,255,255,255);
	td->purple = SDL_MapRGB(td->surf->format,255,  0,255);	
		
	/* Turn on a border, thickness specified by BORDER_THICKNESS */
	td->border_thick = BORDER_THICKNESS;

	/* make the default terminal border color to be white */
	td->border_color = td->white;


	printf("Init-int term: %d\n",i);
#ifdef USE_GRAPHICS
#ifdef USE_TRANSPARENCY
#endif
#endif

	/* Success */
	return (0);
}

/* dumpWindowSettings is responsible for exporting all current
values of the window positions, etc. to the screen, so that 
the user can see what the final values were after tweaking */
void dumpWindowSettings(void)
{
	char name[80];
	char value[8];
	int i;
	
	DB("Dumping settings");
	printf("---------------------------\n");
	/* cycle through each available terminal */
	for (i=0; i<arg_console_count; i++)
	{
		printf("Terminal %d:\n",i);
		/* get the name, and value of each value to dump */
		sprintf(name,"TOME_X_POS_%d",i);
		sprintf(value,"%d",data[i].rect.x);
		printf("%s=%s\n",name,value);

		sprintf(name,"TOME_Y_POS_%d",i);
		sprintf(value,"%d",data[i].rect.y);
		printf("%s=%s\n",name,value);		

		sprintf(name,"TOME_NUM_COLS_%d",i);
		sprintf(value,"%d",data[i].cols);
		printf("%s=%s\n",name,value);		

		sprintf(name,"TOME_NUM_ROWS_%d",i);
		sprintf(value,"%d",data[i].rows);
		printf("%s=%s\n",name,value);		

		/* Simple! */
		printf("\n");
	}
}

/* The main-sdl initialization routine!
This routine processes arguments, opens the SDL
window, loads fonts, etc. */
errr init_sdl(int argc, char **argv)
{
	int i, surface_type;
	char filename[PATH_MAX + 1];
	const char file_sep = '.';
	/* Flags to pass to SDL_SetVideoMode */
	int videoFlags;	

	/* Before sdl_quit could possible be called, need to make sure that the text
	array is zeroed, so that sdl_quit->killFontAndAlphabet() doesn't try to free
	SDL_Surfaces that don't exist ! */
	memset(text,0,sizeof(text));
	
	/* Also, clear out the term order array */
	memset(term_order,0,sizeof(term_order));

	/* initialize SDL */
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		sdl_quit("Video initialization failed!");
	}
	DB("SDL Initialized!");

	/* get video info, to be used for determining if hardware acceleration is
	 available, pixel format, etc.... */
	videoInfo = SDL_GetVideoInfo();
	
	/* Environment calls to retrieve specific settings...
	Note that these can be overridden by the command-line
	arguments that are handled below */
	if(getenv("TOME_CONSOLE_COUNT"))
		arg_console_count = atoi(getenv("TOME_CONSOLE_COUNT"));
	if(getenv("TOME_SCREEN_WIDTH"))
		arg_width = atoi(getenv("TOME_SCREEN_WIDTH"));
	if(getenv("TOME_SCREEN_HEIGHT"))
		arg_height = atoi(getenv("TOME_SCREEN_HEIGHT"));
	if(getenv("TOME_SCREEN_BPP"))
		arg_bpp = atoi(getenv("TOME_SCREEN_BPP"));
	if(getenv("TOME_FONT_SIZE"))
		arg_font_size = atoi(getenv("TOME_FONT_SIZE"));
	
	/* Argument handling routine;
	the argv pointer is already pointing at the '--'
	argument, so just start from there, parsing each
	option as it comes along */
	for (i=1; i < argc ; i++)
	{
		/* Set the number of consoles to handle
		(ie the number of windows) */
		if (0 == strcmp(argv[i], "-n"))
		{
			if (++i == argc)
			{
				printf("Argument missing for option -n\n");
				return -1;
			}
			
			arg_console_count = atoi(argv[i]);
			if (arg_console_count <= 0 || \
				arg_console_count > MAX_CONSOLE_COUNT)
			{
				printf("Invalid console count given.\n");
				arg_console_count = 1;
			}
		}
		/* Set the SDL window/screen width in pixels */
		else if (0 == strcmp(argv[i], "-w"))
	 	{
			if (++i == argc)
			{
				printf("Argument missing for option -w\n");
				return -1;
			}
			
			arg_width = atoi(argv[i]);
		}
		/* Set the SDL window/screen height in pixels */
		else if (0 == strcmp(argv[i], "-h"))
		{
			if (++i == argc)
			{
				printf("Argument missing for option -h\n");
				return -1;
			}
		
			arg_height = atoi(argv[i]);
		}
		/* Set the SDL window/screen color depth
		(in bits per pixel -- only 8,16,32 are okay) */
		else if (0 == strcmp(argv[i], "-bpp"))
		{
			if (++i == argc)
			{
				printf("Argument missing for option -bpp\n");
				return -1;
			}
			
			arg_bpp = atoi(argv[i]);
			if ( (arg_bpp != 8) && (arg_bpp != 16) \
				&& (arg_bpp != 24) && (arg_bpp != 32) )
			{
				printf("Invalid color depth. Must be either 8, 16, or 32 bpp!\n");
				return -1;
			}
		}
		/* see if new graphics are requested...*/
		else if (0 == strcmp(argv[i], "-g"))
		{
			printf("New graphics (16x16) enabled!\n");
			arg_graphics_type = GRAPHICS_16x16;
		}
		/* see if old graphics are requested...*/
		else if (0 == strcmp(argv[i], "-o"))
		{
			printf("Old graphics (8x8) enabled!\n");
			arg_graphics_type = GRAPHICS_8x8;
		}
		
		/* see if double width tiles are requested */
		else if (0 == strcmp(argv[i], "-b"))
		{
			/* do nothing for now */
			/* arg_double_width = TRUE; */
		}
		/* switch into full-screen at startup */
		else if (0 == strcmp(argv[i], "-fs"))
		{
			DB("Full-screen enabled!");
			arg_full_screen = TRUE;
		}
		/* change the font size */
		else if (0 == strcmp(argv[i], "-s"))
		{
			if (++i == argc)
			{
				printf("Argument missing for option -s\n");
				printf("Please specify font size!\n");
				return -1;
			}
			
			arg_font_size = atoi(argv[i]);
		}
		/* change the font to use */
		else if (0 == strcmp(argv[i], "-f"))
		{
			/* Can we please not be so MS Windows-specific?  One of the main goals
			   of SDL in ToME was to be more portable.  These file name hacks are
			   only the idiom of that one OS, though. -- Neil */
			DB("Getting font name");
			if (++i == argc)
			{
				printf("Argument missing for option -f\n");
				printf("Please specify a true-type font found in /lib/xtra/font!\n");
				return -1;
			}
			
			/* tokenize the font name so that no .ttf extension
			is required */
			strcpy(arg_font_name,\
				strtok(argv[i],&file_sep));
			
			/* and append the extension */
			strcat(arg_font_name,".ttf");

			/* print a little debug message, so
			user sees what font was actually selected */
			printf("\tUsing font: %s\n",arg_font_name);
			
			/* maybe check to see if file is even
			existant in /lib/xtra/font */
		}
		
	} /* end argument handling */

	/* Make sure that the engine will shutdown SDL properly*/
	quit_aux = sdl_quit;

	/* Use the ToME logo and set the window name */
	filename[PATH_MAX] = 0;
	path_build(filename, PATH_MAX, ANGBAND_DIR_XTRA, "graf/icon.png");
	SDL_WM_SetIcon(IMG_Load(filename), 0);
	SDL_WM_SetCaption("ToME", "tome");

	/* SDL video settings, dependent on whether hardware is available */
	if (videoInfo->hw_available)
		videoFlags = SDL_HWSURFACE;
	else
		videoFlags = SDL_SWSURFACE;

	/* now set the video mode that has been configured */
	screen = SDL_SetVideoMode( arg_width, arg_height, arg_bpp, videoFlags );

	/* Verify there is a surface */
	if ( !screen )
	{
		DB("No screen!");
		sdl_quit("Failed to set SDL Surface.");
	}

	DB("Video Mode Set!");
	
	/* now switch into full screen if asked for */
	if (arg_full_screen)
		SDL_WM_ToggleFullScreen(screen);
	
	DB("SDL Window Created!");

	/* Now ready the fonts! */

	DB("initializing SDL_ttf");
	if(TTF_Init()==-1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		sdl_quit("Bah");
	}

	DB("loading font...");

	/* load and render the font */
	loadAndRenderFont(arg_font_name,arg_font_size);

	/* Graphics! ----
	If graphics are selected, then load graphical tiles! */
	if (arg_graphics_type != NO_GRAPHICS)
	{
		/* load graphics tiles */
	}
	
	/* Initialize the working surface and crayon surface used for rendering
	 text in different colors. */

	worksurf = createSurface(t_width,t_height);
	crayon = createSurface(t_width,t_height);
	
	/* The working surface will blit using alpha values... */
	SDL_SetAlpha(worksurf,SDL_SRCALPHA,0);

	/* Set up the colors using the great little color macros! */
	color_data[0]  = BLACK;
	color_data[1]  = WHITE;
	color_data[2]  = MID_GREY;
	color_data[3]  = BRIGHT_ORANGE;
	color_data[4]  = RED;
	color_data[5]  = GREEN;
	color_data[6]  = BRIGHT_BLUE;
	color_data[7]  = DARK_ORANGE;
	color_data[8]  = DARK_GREY;
	color_data[9]  = BRIGHT_GREY;
	color_data[10] = PURPLE;
	color_data[11] = YELLOW;
	color_data[12] = BRIGHT_RED;
	color_data[13] = BRIGHT_GREEN;
	color_data[14] = AQUAMARINE;
	color_data[15] = BROWN;

	/* And setup the cursor, using the default color...
	XXX - in the future, this should (and will) be loaded from prefs */
	createCursor(DEF_CURSOR_COLOR);

	/* Initialize the windows, or whatever that means in this case.
	Do this in reverse order so that the main window is on top.*/
	suspendUpdate = TRUE;	/* draw everything at the end */
	i = arg_console_count;
	while (i--)
	{
		term_data *td = &data[i];

		/* Initialize the term_data */
		term_data_init(td, i);

		/* Save global entry */
		angband_term[i] = Term;
		
		/* Add into term_order */
		term_order[i] = td;
		
	}
	
	/* And setup the basic screen colors -- these are keyed to the format of
	the main terminal surface */
	screen_black  = SDL_MapRGB(screen->format,  0,  0,  0);

	suspendUpdate = FALSE; /* now draw everything */
	redrawAllTerminals();
	/*SDL_REDRAW_SCREEN;*/
	
	/* now that the windows have been set, their settings can
	be dumped upon quit! */
	window_properties_set = TRUE;

	/* Enable UNICODE keysyms - needed for current eventHandling routine */
	SDL_EnableUNICODE(1);
	
	/* Enable key repeat! */
	SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY,SDL_DEFAULT_REPEAT_INTERVAL);

	#ifdef USE_ISO
	DB("Isometric view uses always graphics mode.\n");
	use_graphics = TRUE;

	
	/* Hajo: allocate memory for output data */
	/* These arrays are read by the iso-view and written from this file */
	iso_cp = halloc(data[0].t.wid, data[0].t.hgt);
	iso_ap = halloc(data[0].t.wid, data[0].t.hgt);
	iso_ctp = halloc(data[0].t.wid, data[0].t.hgt);
	iso_atp = halloc(data[0].t.wid, data[0].t.hgt);
	iso_cep = halloc(data[0].t.wid, data[0].t.hgt);
	iso_aep = halloc(data[0].t.wid, data[0].t.hgt);

	// Hmm, no ANGBAND_SYS in old iso-code
	// if I change this I don't have to load the *.prf manually?
	// 
	// seems not to work for the following:	
	/* Hajo: set mode */
	ANGBAND_GRAF = "iso";

	/* Hajo: init view */
	init_adaptor();
	
	center_player = TRUE;
#endif /* USE_ISO */

#ifdef USE_ISO
	// Juergen: HACK, but this all is just for testing ...
	data[0].t.higher_pict = TRUE;
#endif

	/* main-sdl initialized! */
	return 0;
}

#endif

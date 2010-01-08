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
#define DEF_FONT_SIZE 16
#define DEF_FONT_FILE "lib/xtra/font/VeraMono.ttf"

/* The font properties that may perhaps be changed at runtime,
due to environmental variables, preference files, or in-program
commands.*/
static int arg_font_size = DEF_FONT_SIZE;
static char arg_font_name[64] = DEF_FONT_FILE;

/**************/

/* The number of term_data structures to set aside mem for */
#define MAX_CONSOLE_COUNT 8

/* The number of consoles that are actually being used.
This number could be changed via preference files, environmental
variables, command-line arguments, or possibly even in-game
keypresses or menu-selections. */
static int arg_console_count = 1;

/* When rendering multiple terminals, each is drawn with a
surrounding border. These values control the width of this
border and also the color to use when drawing it. */
#define BORDER_THICKNESS 1
static int border_color = 0;

/**************/

/* some miscellaneous settings which have not been dealt
with yet */
static bool arg_old_graphics = FALSE;
static bool arg_double_width = FALSE;

/* not dealt with yet (although full screen toggle
is available using Alt-Enter) */
static bool arg_full_screen = FALSE;


/*************************************************
 GLOBAL SDL-ToME VARIABLES
 *************************************************/

/* the main screen to draw to */
static SDL_Surface *screen;

/* some helper surfaces that are used for rendering 
characters */
static SDL_Surface *worksurf;
static SDL_Surface *crayon;

/* the array of pre-rendered characters
(see loadAndRenderFont() below) */
SDL_Surface *text[128];

/* the actual TTF_Font used (XXX should get rid of this)*/
TTF_Font *font=0;

/* the width and height of the uniformly-sized pre-rendered
characters */
int t_width = 1, t_height = 1;


/*************************************************
 COLOR SETUP
 *************************************************/
int screen_black;
int screen_white;

static int color_data[16];
/* The following macro is for color defining...
 Note that the color is fully opaque... */
#define COLOR(r,g,b) \
	SDL_MapRGBA(crayon->format,r,g,b,SDL_ALPHA_OPAQUE)

/*These color macros will setup the colors to use, but must be called after
 the SDL video has been set. That way SDL can correct for any funky video
 setttings. */

#define BLACK			COLOR(0x00,0x00,0x00)
#define WHITE			COLOR(0xff,0xff,0xff)
#define MID_GREY		COLOR(0x80,0x80,0x80)
#define BRIGHT_ORANGE	COLOR(0xff,0x80,0x00)
#define RED				COLOR(0xc0,0x00,0x00)
#define GREEN			COLOR(0x00,0x80,0x40)
#define BRIGHT_BLUE		COLOR(0x00,0x00,0xff)
#define DARK_ORANGE		COLOR(0x80,0x40,0x00)
#define DARK_GREY		COLOR(0x40,0x40,0x40)
#define BRIGHT_GREY		COLOR(0xc0,0xc0,0xc0)
#define PURPLE			COLOR(0xff,0x00,0xff)
#define YELLOW			COLOR(0xff,0xff,0x00)
#define BRIGHT_RED		COLOR(0xff,0x00,0x00)
#define BRIGHT_GREEN	COLOR(0x00,0xff,0x00)
#define AQUAMARINE		COLOR(0x00,0xff,0xff)
#define BROWN			COLOR(0xc0,0x80,0x40)

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
	uint pos_x, pos_y;		/* upper left corner of rendering box */
	uint size_w, size_h;	/* width, height of rendering box */

	bool has_border;		/* whether this sub-window has a border or not */
	uint border_thick;		/* thickness of border to draw around window */
#ifdef USE_GRAPHICS
#ifdef USE_TRANSPARENCY
#endif
#endif
};

/* The array of term data structures */
static term_data data[MAX_CONSOLE_COUNT];

/*************************************************
 FILE-SPECIFIC MACROS
 *************************************************/

/* Debug macros! */
#define DB(str) \
	printf("main-sdl: %s\n",str);

/*************************************************
 COLOR SETUP
 *************************************************/

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
	/* And now for the default quit behavior */
	quit_aux = 0;
	quit(string);
}

/*************************************************
 FONT SUPPORT FUNCTIONS
 *************************************************/

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
	SDL_Color base_color = {255,255,255,255};
	SDL_Surface *temp_surf;
	SDL_Rect tgt = {0,0,0,0};

	/* Assuming that the filename is valid,
	open the font (pointer is global var)*/
	if (fname == NULL)
		sdl_quit("Gimme a font to load!");
	font = TTF_OpenFont(fname,size);
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
		text[i] = SDL_CreateRGBSurface(SDL_HWSURFACE,t_width,\
			t_height,16,0xf000,0x0f00,0x00f0,0x000f);
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

/***********************************************/

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
			sdl_quit("Quitting!\n");
			break;
		}
	default:
		{
			break;
		}
	}
}

static errr Term_xtra_sdl(int n, int v)
{
	static SDL_Event event;
	term_data *td;
	char buf[1024];

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
			/* Perform a full screen clear; redraw the sub-window borders.*/
			static SDL_Rect base;
			base.x = 0;
			base.y = 0;
			base.w = arg_width;
			base.h = arg_height;

			td = (term_data*)(Term->data);

			/* Lock the screen */
			if (SDL_MUSTLOCK(screen) ){
				if (SDL_LockSurface(screen) < 0) {
					printf("Can't lock the screen: %s\n", SDL_GetError());
					exit(1);
				}
			}

			/* blank the screen area */
			SDL_FillRect(screen, &base, screen_black);

			/* Unlock the screen */
			if (SDL_MUSTLOCK(screen) ){
				SDL_UnlockSurface(screen);
			}

			/* And... UPDATE the whole screen */
			SDL_Flip(screen);

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
#endif /* USE_ISO */
			return (1);
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

#define TYPECOLOR(i) 	printf("  R:%d\tG:%d\tB:%d\tA:%d\t\n",\
	color_data[i]>>24,(color_data[i]&0x00ff0000)>>16,\
	(color_data[i]&0x0000ff00)>>8,(color_data[i]&0x000000ff));
/*
 * Display the cursor
 *
 * This routine should display the cursor at the given location
 * (x,y) in some manner.  On some machines this involves actually
 * moving the physical cursor, on others it involves drawing a fake
 * cursor in some form of graphics mode.  Note the "soft_cursor"
 * flag which tells "z-term.c" to treat the "cursor" as a "visual"
 * thing and not as a "hardware" cursor.
 *
 * You may assume "valid" input if the window is properly sized.
 *
 * You may use the "Term_grab(x, y, &a, &c)" function, if needed,
 * to determine what attr/char should be "under" the new cursor,
 * for "inverting" purposes or whatever.
 */
static errr Term_curs_sdl(int x, int y)
{
	term_data *td = (term_data*)(Term->data);
	DB("Term_curs_sdl");
	/* XXX XXX XXX */
#ifdef USE_ISO
	highlite_spot(x, y);
#endif

	/* Success */
	return (0);
}


/*
 * Erase some characters
 *
 * This function should erase "n" characters starting at (x,y).
 *
 * You may assume "valid" input if the window is properly sized.
 */
static errr Term_wipe_sdl(int x, int y, int n)
{
	static SDL_Rect base;
	term_data *td = (term_data*)(Term->data);
	DB("Wiping");
	/* calculate boundaries of the area to clear */
	base.x = td->pos_x + x*t_width;
	base.y = td->pos_y + y*t_height;
	base.w = n*t_width;
	base.h = t_height;

	/* Lock the screen */
	if (SDL_MUSTLOCK(screen) ){
		if (SDL_LockSurface(screen) < 0) {
			printf("Can't lock the screen: %s\n", SDL_GetError());
			sdl_quit("Bah");
		}
	}

	/* blank the screen area */
	SDL_FillRect(screen, &base, screen_black);

	/* Unlock the screen */
	if (SDL_MUSTLOCK(screen) ){
		SDL_UnlockSurface(screen);
	}

	/* And... UPDATE the rectangle we just wrote to! */
	SDL_UpdateRects(screen,1,&base);

	/* Success */
	return (0);
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
	term_data *td = (term_data*)(Term->data);
	static SDL_Rect base;
	SDL_Rect base_back;
	int i = n;
	char old = 0;

	/* calculate place to clear off and draw to */
	base.x = td->pos_x + x*t_width;
	base.y = td->pos_y + y*t_height;
	base.w = n*t_width;
	base.h = t_height;

	base_back = base;

	/* Lock the screen */
	if (SDL_MUSTLOCK(screen) ){
		if (SDL_LockSurface(screen) < 0) {
			printf("Can't lock the screen: %s\n", SDL_GetError());
			sdl_quit("Bah");
		}
	}

	/* blank the screen area */
	SDL_FillRect(screen, &base, screen_black);

	/* Unlock the screen */
	if (SDL_MUSTLOCK(screen) ){
		SDL_UnlockSurface(screen);
	}

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
			SDL_BlitSurface(worksurf,NULL,screen,&base);
		} else {
			/* copy the desired character onto working surface */
			SDL_BlitSurface(text[*cp],NULL,worksurf,NULL);
			/* color our crayon surface with the desired color */
			SDL_FillRect(crayon,NULL,color_data[a&0x0f]);
			/* apply the color to the character on the working surface */
			SDL_BlitSurface(crayon,NULL,worksurf,NULL);
			/* and blit it onto our screen! */
			SDL_BlitSurface(worksurf,NULL,screen,&base);
		}
		/* Move to the next position */
		base.x += t_width;
		/* Store the old character */
		old = *cp;
	/* Increment the character pointer */
		cp++;
	}

	// And... UPDATE the rectangle we just wrote to!
	SDL_UpdateRects(screen,1,&base_back);

#ifdef USE_ISO
	if (a < 16)
	{
		set_spots(x, y, n, TRUE);
	}
	else
	{
		set_spots(x, y, n, FALSE);
	}
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

static errr term_data_init(term_data *td, int i)
{
	term *t = &(td->t);
	int x = 0;
	int y = 0;
	int cols = 80;
	int rows = 24;

	/* Initialize the term */
	// gets: pointer to address, number of columns, number of rows, number
	//   of keypresses to queue up (guess 24?)
	term_init(t, cols, rows, 24);

	/* Use a "soft" cursor */
	t->soft_cursor = TRUE;

	// Picture routine flags
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

	// *** Initialize the rest of the term_data stuff....
	td->name = angband_term_name[i];// name of this term window

	td->rows = rows;
	td->cols = cols;
	td->pos_x = x;
	td->pos_y = y;


	td->size_w = cols*t_width;
	td->size_h = rows*t_height;

	/* Turn on a border, thickness specified by BORDER_THICKNESS */
	td->has_border = TRUE;
	td->border_thick = BORDER_THICKNESS;

#ifdef USE_GRAPHICS
#ifdef USE_TRANSPARENCY
#endif
#endif

	/* Success */
	return (0);
}

#ifdef PRIVATE_USER_PATH

/*
 * Check and create if needed the directory dirpath -- copied from main.c
 */
bool private_check_user_directory(cptr dirpath)
{
	/* Is this used anywhere else in *bands? */
	struct stat stat_buf;

	int ret;

	/* See if it already exists */
	ret = stat(dirpath, &stat_buf);

	/* It does */
	if (ret == 0)
	{
		/* Now we see if it's a directory */
		if ((stat_buf.st_mode & S_IFMT) == S_IFDIR) return (TRUE);

		/*
		 * Something prevents us from create a directory with
		 * the same pathname
		 */
		return (FALSE);
	}

	/* No - this maybe the first time. Try to create a directory */
	else
	{
		/* Create the ~/.ToME directory */
		ret = mkdir(dirpath, 0700);

		/* An error occured */
		if (ret == -1) return (FALSE);

		/* Success */
		return (TRUE);
	}
}

/*
 * Check existence of ".ToME/" directory in the user's
 * home directory or try to create it if it doesn't exist.
 * Returns FALSE if all the attempts fail.
 */
static bool check_create_user_dir(void)
{
	char dirpath[1024];
	char versionpath[1024];
	char savepath[1024];

	/* Get an absolute path from the filename */
	path_parse(dirpath, 1024, PRIVATE_USER_PATH);
	strcpy(versionpath, dirpath);
	strcat(versionpath, USER_PATH_VERSION);
	strcpy(savepath, versionpath);
	strcat(savepath, "/save");

	return private_check_user_directory(dirpath) && private_check_user_directory(versionpath) && private_check_user_directory(savepath);
}

#endif /* PRIVATE_USER_PATH */

/*
 * Init some stuff - copied from main.c
 */
static void init_stuff(void)
{
	char path[1024];

	cptr tail;

	/* Get the environment variable */
	tail = getenv("TOME_PATH");

	/* Use the angband_path, or a default */
#ifndef ENABLE_BINRELOC
	strcpy(path, tail ? tail : DEFAULT_PATH);
#else /* Runtime lookup of location */
	strcpy(path, br_strcat(DATADIR, "/tome/lib"));
#endif

	/* Hack -- Add a path separator (only if needed) */
	if (!suffix(path, PATH_SEP)) strcat(path, PATH_SEP);

	/* Initialize */
	init_file_paths(path);
}


errr init_sdl(int argc, char **argv)
{
	return 0;
}

int main(int argc, char *argv[])
{
	int i;

	bool done = FALSE;

	bool new_game = FALSE;

	int show_score = 0;

	cptr mstr = NULL;

	bool args = TRUE;

	float gamma;
	char filename[PATH_MAX + 1];
	/* Flags to pass to SDL_SetVideoMode */
	int videoFlags;
	/* this holds some info about our display */
	const SDL_VideoInfo *videoInfo;


#ifdef CHECK_MEMORY_LEAKS
	GC_find_leak = 1;
#endif /* CHECK_MEMORY_LEAKS */


	/* Save the "program name" XXX XXX XXX */
	argv0 = argv[0];


#ifdef USE_286
	/* Attempt to use XMS (or EMS) memory for swap space */
	if (_OvrInitExt(0L, 0L))
	{
		_OvrInitEms(0, 0, 64);
	}
#endif


#ifdef SET_UID

	/* Default permissions on files */
	(void)umask(022);

#endif /* SET_UID */


	/* Get the file paths */
	init_stuff();


#ifdef SET_UID

	/* Get the user id (?) */
	player_uid = getuid();

#ifdef VMS
	/* Mega-Hack -- Factor group id */
	player_uid += (getgid() * 1000);
#endif

# ifdef SAFE_SETUID

# ifdef _POSIX_SAVED_IDS

	/* Save some info for later */
	player_euid = geteuid();
	player_egid = getegid();

# endif

# if 0	/* XXX XXX XXX */

	/* Redundant setting necessary in case root is running the game */
	/* If not root or game not setuid the following two calls do nothing */

	if (setgid(getegid()) != 0)
	{
		sdl_quit("setgid(): cannot set permissions correctly!");
	}

	if (setuid(geteuid()) != 0)
	{
		sdl_quit("setuid(): cannot set permissions correctly!");
	}

# endif  /* XXX XXX XXX */

# endif  /* SAFE_SETUID */

#endif /* SET_UID */


#ifdef SET_UID

	/* Please note that the game is still running in the game's permission */

	/* Initialize the "time" checker */
	if (check_time_init() || check_time())
	{
		sdl_quit("The gates to Angband are closed (bad time).");
	}

	/* Initialize the "load" checker */
	if (check_load_init() || check_load())
	{
		sdl_quit("The gates to Angband are closed (bad load).");
	}


	/*
	 * Become user -- This will be the normal state for the rest of the game.
	 *
	 * Put this here because it's totally irrelevant to single user operating
	 * systems, as witnessed by huge number of cases where these functions
	 * weren't used appropriately (at least in this variant).
	 *
	 * Whenever it is necessary to open/remove/move the files in the lib folder,
	 * this convention must be observed:
	 *
	 *    safe_setuid_grab();
	 *
	 *    fd_open/fd_make/fd_kill/fd_move which requires game's permission,
	 *    i.e. manipulating files under the lib directory
	 *
	 *    safe_setuid_drop();
	 *
	 * Please never ever make unmatched calls to these grab/drop functions.
	 *
	 * Please note that temporary files used by various information commands
	 * and ANGBAND_DIR_USER files shouldn't be manipulated this way, because
	 * they reside outside of the lib directory on multiuser installations.
	 * -- pelpel
	 */
	safe_setuid_drop();


	/* Acquire the "user name" as a default player name */
	user_name(player_name, player_uid);


#ifdef PRIVATE_USER_PATH

	/*
	 * On multiuser systems, users' private directories are
	 * used to store pref files, chardumps etc.
	 */
	{
		bool ret;

		/* Create a directory for the user's files */
		ret = check_create_user_dir();

		/* Oops */
		if (ret == FALSE) sdl_quit("Cannot create directory " PRIVATE_USER_PATH);
	}

#endif /* PRIVATE_USER_PATH */

#endif /* SET_UID */

	/* Before sdl_quit could possible be called, need to make sure that the text
	array is zeroed, so that sdl_quit->killFontAndAlphabet() doesn't try to free
	SDL_Surfaces that don't exist ! */
	memset(text,0,sizeof(text));

	/* Initialize the SDL window*/
	filename[PATH_MAX] = 0;

	/* initialize SDL */
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		sdl_quit("Video initialization failed!");
	}
	DB("SDL Initialized!");

	/* Skip to our arguments -- (Neil)*/
	/*	for (i = 1; (i < argc) && (0 != strcmp(argv[i], "--")); ++i); */
	/* Handle our arguments -- (Neil)*/
	/*	for (++i; i < argc ; ++i)
	 {
	 if (0 == strcmp(argv[i], "-n"))
	 {
	 if (++i == argc)
	 {
	 printf("Argument missing for option -n\n");
	 return -1;
	 }

	 arg_console_count = atoi(argv[i]);
	 if (arg_console_count <= 0 || arg_console_count > MAX_CONSOLE_COUNT)
	 {
	 printf("Invalid console count given.\n");
	 arg_console_count = 1;
	 }
	 }
	 else if (0 == strcmp(argv[i], "-o"))
	 {
	 arg_old_graphics = TRUE;
	 }
	 else if (0 == strcmp(argv[i], "-b"))
	 {
	 arg_double_width = TRUE;
	 }
	 else if (0 == strcmp(argv[i], "-w"))
	 {
	 if (++i == argc)
	 {
	 printf("Argument missing for option -w\n");
	 return -1;
	 }

	 arg_width = atoi(argv[i]);
	 }
	 else if (0 == strcmp(argv[i], "-h"))
	 {
	 if (++i == argc)
	 {
	 printf("Argument missing for option -h\n");
	 return -1;
	 }

	 arg_height = atoi(argv[i]);
	 }
	 else if (0 == strcmp(argv[i], "-fs"))
	 {
	 arg_full_screen = TRUE;
	 }
	 else if (0 == strcmp(argv[i], "-bpp"))
	 {
	 if (++i == argc)
	 {
	 printf("Argument missing for option -bpp\n");
	 return -1;
	 }

	 arg_bpp = atoi(argv[i]);
	 }
	 }
	 */


	/* Now for the meat of the initialization -- (Neil)*/
	quit_aux = sdl_quit;

	/* Window Manager stuff -- (Neil)*/
	path_build(filename, PATH_MAX, ANGBAND_DIR_XTRA, "graf/icon.png");
	SDL_WM_SetIcon(IMG_Load(filename), 0);
	SDL_WM_SetCaption("ToME", "tome");

	/* how about a hardware surface with hardware palette?
	XXX XXX XXX should probably be chosen at compile-time! */
	videoFlags = SDL_HWSURFACE | SDL_HWPALETTE;

	/* XXX XXX XXX */
	if(getenv("TOME_SCREEN_WIDTH")) arg_width = atoi(getenv("TOME_SCREEN_WIDTH"));
	if(getenv("TOME_SCREEN_HEIGHT")) arg_height = atoi(getenv("TOME_SCREEN_HEIGHT"));
	if(getenv("TOME_SCREEN_BPP")) arg_bpp = atoi(getenv("TOME_SCREEN_BPP"));

	/* get a SDL surface */
	screen = SDL_SetVideoMode( arg_width, arg_height, arg_bpp, videoFlags );

	DB("Video Mode Set!");

	/* Verify there is a surface */
	if ( !screen )
	{
		DB("No screen!");
		sdl_quit("Failed to set SDL Surface.");
	}

	DB("SDL Window Created!");

	/* Now ready the fonts! */

	DB("initializing SDL_ttf");
	if(TTF_Init()==-1) {
		printf("TTF_Init: %s\n", TTF_GetError());
		sdl_quit("Bah");
	}

	DB("loading font...");
	/* XXX centralize these environment calls*/
	if(getenv("TOME_FONT_SIZE")) arg_font_size = atoi(getenv("TOME_FONT_SIZE"));

	/* load and render the font */
	loadAndRenderFont(arg_font_name,arg_font_size);

	/* Initialize the working surface and crayon surface used for rendering
	 text in different colors... */
	worksurf = SDL_CreateRGBSurface(SDL_HWSURFACE,t_width,\
		t_height,16,0xf000,0x0f00,0x00f0,0x000f);
	crayon = SDL_CreateRGBSurface(SDL_HWSURFACE,t_width,\
		t_height,16,0xf000,0x0f00,0x00f0,0x000f);

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

	/* And setup the screen black color */
	screen_black = SDL_MapRGB(screen->format,0,0,0);
	screen_white = SDL_MapRGB(screen->format,255,255,255);

	/* Initialize the windows, or whatever that means in this case */
	for (i = 0; i < MAX_CONSOLE_COUNT; i++)
	{
		term_data *td = &data[i];

		/* Initialize the term_data */
		term_data_init(td, i);
		/* Save global entry */
		angband_term[i] = Term;
	}

	/* Enable UNICODE keysyms */
	SDL_EnableUNICODE(1);

	/* By setting this value, 'pref-sdl.prf' will be loaded on start.
	 Since this contains mappings for various keys, this is important! */
	ANGBAND_SYS = "sdl";
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

	printf("Signals?\n");

	/* Catch nasty signals */
	signals_init();

	printf("Initialize Angband!\n");
	/* Initialize */
	init_angband();

	printf("Angband Initialized!\n");

	/* Hack -- If requested, display scores and quit */
	if (show_score > 0) display_scores(0, show_score);

	/* Wait for response */
	pause_line(23);

	printf("Play the game!\n");
#ifdef USE_ISO
	// Juergen: HACK, but this all is just for testing ...
	data[0].t.higher_pict = TRUE;
#endif

	/* Play the game */
	play_game(new_game);

	/* Quit */
	sdl_quit("Game over, man");

	/* Exit */
	return (0);

}

#endif

/*
 * File: main-lsl.c
 * Purpose: Support for Linux-SVGALIB Angband
 * Original Author: Jon Taylor (taylorj@gaia.ecs.csus.edu)
 * Update by: Dennis Payne (dulsi@identicalsoftware.com)
 * Version: 1.4.0, 12/05/99
 *
 * Large amounts of code rewritten by Steven Fuerst. 20/04/2001
 *
 * It now uses a hacked-up version of the X11 bmp-loading code.
 * (Preparing to use 256 colour 16x16 mode)
 */

#include "angband.h"

#ifdef USE_LSL

/* Standard C header files */
#include <stdio.h>
#include <stdlib.h>

/* SVGAlib header files */
#include <vga.h>
#include <vgagl.h>
#include <vgakeyboard.h>
#include <zlib.h>

#define COLOR_OFFSET 240

/* Hack - Define font/graphics cell width and height */
#define CHAR_W 8
#define CHAR_H 13

/* Global palette */
static byte *pal = NULL;

#ifdef USE_GRAPHICS

/*
 * The Win32 "BITMAPFILEHEADER" type.
 */
typedef struct BITMAPFILEHEADER
{
	u16b bfType;
	u32b bfSize;
	u16b bfReserved1;
	u16b bfReserved2;
	u32b bfOffBits;
}
BITMAPFILEHEADER;


/*
 * The Win32 "BITMAPINFOHEADER" type.
 */
typedef struct BITMAPINFOHEADER
{
	u32b biSize;
	u32b biWidth;
	u32b biHeight;
	u16b biPlanes;
	u16b biBitCount;
	u32b biCompresion;
	u32b biSizeImage;
	u32b biXPelsPerMeter;
	u32b biYPelsPerMeter;
	u32b biClrUsed;
	u32b biClrImportand;
}
BITMAPINFOHEADER;

/*
 * The Win32 "RGBQUAD" type.
 */
typedef struct RGBQUAD
{
	unsigned char b, g, r;
	unsigned char filler;
}
RGBQUAD;


/*** Helper functions for system independent file loading. ***/

static byte get_byte(FILE *fff)
{
	/* Get a character, and return it */
	return (getc(fff) & 0xFF);
}

static void rd_byte(FILE *fff, byte *ip)
{
	*ip = get_byte(fff);
}

static void rd_u16b(FILE *fff, u16b *ip)
{
	(*ip) = get_byte(fff);
	(*ip) |= ((u16b)(get_byte(fff)) << 8);
}

static void rd_u32b(FILE *fff, u32b *ip)
{
	(*ip) = get_byte(fff);
	(*ip) |= ((u32b)(get_byte(fff)) << 8);
	(*ip) |= ((u32b)(get_byte(fff)) << 16);
	(*ip) |= ((u32b)(get_byte(fff)) << 24);
}


/*
 * Read a Win32 BMP file.
 *
 * Assumes that the bitmap has a size such that no padding is needed in
 * various places.  Currently only handles bitmaps with 3 to 256 colors.
 */
static byte *ReadBMP(char *Name, int *bw, int *bh)
{
	FILE *f;

	BITMAPFILEHEADER fileheader;
	BITMAPINFOHEADER infoheader;

	byte *Data;

	int ncol;

	int total;

	int i;

	u16b x, y;

	/* Open the BMP file */
	f = fopen(Name, "r");

	/* No such file */
	if (!f)
	{
		quit ("No bitmap to load!");
	}

	/* Read the "BITMAPFILEHEADER" */
	rd_u16b(f, &(fileheader.bfType));
	rd_u32b(f, &(fileheader.bfSize));
	rd_u16b(f, &(fileheader.bfReserved1));
	rd_u16b(f, &(fileheader.bfReserved2));
	rd_u32b(f, &(fileheader.bfOffBits));

	/* Read the "BITMAPINFOHEADER" */
	rd_u32b(f, &(infoheader.biSize));
	rd_u32b(f, &(infoheader.biWidth));
	rd_u32b(f, &(infoheader.biHeight));
	rd_u16b(f, &(infoheader.biPlanes));
	rd_u16b(f, &(infoheader.biBitCount));
	rd_u32b(f, &(infoheader.biCompresion));
	rd_u32b(f, &(infoheader.biSizeImage));
	rd_u32b(f, &(infoheader.biXPelsPerMeter));
	rd_u32b(f, &(infoheader.biYPelsPerMeter));
	rd_u32b(f, &(infoheader.biClrUsed));
	rd_u32b(f, &(infoheader.biClrImportand));

	/* Verify the header */
	if (feof(f) ||
	                (fileheader.bfType != 19778) ||
	                (infoheader.biSize != 40))
	{
		quit_fmt("Incorrect BMP file format %s", Name);
	}

	/* The two headers above occupy 54 bytes total */
	/* The "bfOffBits" field says where the data starts */
	/* The "biClrUsed" field does not seem to be reliable */
	/* Compute number of colors recorded */
	ncol = (fileheader.bfOffBits - 54) / 4;

	for (i = 0; i < ncol; i++)
	{
		RGBQUAD clrg;

		/* Read an "RGBQUAD" */
		rd_byte(f, &(clrg.b));
		rd_byte(f, &(clrg.g));
		rd_byte(f, &(clrg.r));
		rd_byte(f, &(clrg.filler));

		/* Analyze the color */
		pal[i * 3] = clrg.b;
		pal[i * 3 + 1] = clrg.g;
		pal[i * 3 + 2] = clrg.r;
	}

	/* Look for illegal bitdepths. */
	if ((infoheader.biBitCount == 1) || (infoheader.biBitCount == 24))
	{
		quit_fmt("Illegal biBitCount %d in %s",
		         infoheader.biBitCount, Name);
	}

	/* Determine total bytes needed for image */
	total = infoheader.biWidth * (infoheader.biHeight + 2);

	/* Allocate image memory */
	C_MAKE(Data, total, byte);

	for (y = 0; y < infoheader.biHeight; y++)
	{
		int y2 = infoheader.biHeight - y - 1;

		for (x = 0; x < infoheader.biWidth; x++)
		{
			int ch = getc(f);

			/* Verify not at end of file XXX XXX */
			if (feof(f)) quit_fmt("Unexpected end of file in %s", Name);

			if (infoheader.biBitCount == 8)
			{
				Data[x + y2 * infoheader.biWidth] = ch;
			}
			else if (infoheader.biBitCount == 4)
			{
				Data[x + y2 * infoheader.biWidth] = ch / 16;
				x++;
				Data[x + y2 * infoheader.biWidth] = ch % 16;
			}
		}
	}

	fclose(f);

	/* Save the size for later */
	*bw = infoheader.biWidth;
	*bh = infoheader.biHeight;

	return (Data);
}

#endif /* USE_GRAPHICS */


/* The main "term" structure */
static term term_screen_body;

/* The visible and virtual screens */
GraphicsContext *screen;
GraphicsContext *buffer;

/* The font data */
static void *font;

/* Initialize the screen font */
static void initfont(void)
{
	gzFile fontfile;
	void *temp;
	long junk;

	if (!(fontfile = gzopen("/usr/lib/kbd/consolefonts/lat1-12.psf.gz", "r")))
	{
		/* Try uncompressed */
		if (!(fontfile = gzopen("/usr/lib/kbd/consolefonts/lat1-12.psf", "r")))
		{
			printf ("Error: could not open font file.  Aborting....\n");
			exit(1);
		}
	}

	/* Junk the 4-byte header */
	gzread(fontfile, &junk, 4);

	/* Initialize font */

	/*
	 * Read in 13 bytes per character, and there are 256 characters
	 * in the font.  This means we need to load 13x256 = 3328 bytes.
	 */
	C_MAKE(temp, 256 * 13, byte);
	gzread(fontfile, temp, 256 * 13);

	/*
	 * I don't understand this code - SF
	 * (Is it converting from 8x13 -> 8x12?) 
	 *
	 * I assume 15 is a colour...
	 */
	font = malloc(256 * 8 * 12 * BYTESPERPIXEL);
	gl_expandfont(8, 12, 15, temp, font);
	gl_setfont(8, 12, font);

	/* Cleanup */
	C_FREE(temp, 256 * 13, byte);
	gzclose(fontfile);
}

/* Initialize palette values for colors 0-15 */
static void setpal(void)
{
	int i;
	gl_setpalette(pal);
	for (i = 0; i < 16; i++)
	{
		gl_setpalettecolor(COLOR_OFFSET + i,
		                   angband_color_table[i][1] >> 2,
		                   angband_color_table[i][2] >> 2,
		                   angband_color_table[i][3] >> 2);
	}
}

/*
 * Check for "events"
 * If block, then busy-loop waiting for event, else check once and exit.
 */
static errr CheckEvents(int block)
{
	int k = 0;

	if (block)
	{
		k = vga_getkey();
		if (k < 1) return (1);
	}
	else
	{
		k = vga_getch();
	}

	Term_keypress(k);
	return (0);
}


/*
 * Low-level graphics routine (assumes valid input)
 * Do a "special thing"
 */
static errr term_xtra_svgalib(int n, int v)
{
	switch (n)
	{
	case TERM_XTRA_EVENT:
		{
			/* Process some pending events */
			if (v) return (CheckEvents (FALSE));
			while (!CheckEvents (TRUE));
			return 0;
		}

	case TERM_XTRA_FLUSH:
		{
			/* Flush all pending events */
			/* Should discard all key presses but unimplemented */
			return 0;
		}

	case TERM_XTRA_CLEAR:
		{
			/* Clear the entire window */
			gl_fillbox (0, 0, 80 * CHAR_W, 25 * CHAR_H, 0);
			return 0;
		}

	case TERM_XTRA_DELAY:
		{
			/* Delay for some milliseconds */
			usleep(1000 * v);
			return 0;
		}
	}
	return 1;
}

/*
 * Low-level graphics routine (assumes valid input)
 * Draws a "cursor" at (x,y)
 */
static errr term_curs_svgalib(int x, int y)
{
	gl_fillbox(x * CHAR_W, y * CHAR_H, CHAR_W, CHAR_H, 15);
	return (0);
}

/*
 * Low-level graphics routine (assumes valid input)
 * Erases a rectangular block of characters from (x,y) to (x+w,y+h)
 */
static errr term_wipe_svgalib(int x, int y, int n)
{
	gl_fillbox(x * CHAR_W, y * CHAR_H, n * CHAR_W, CHAR_H, 0);
	return (0);
}

/*
 * Low-level graphics routine (assumes valid input)
 * Draw n chars at location (x,y) with value s and attribute a
 */
static errr term_text_svgalib(int x, int y, int n, byte a, cptr s)
{
	/* Clear the area */
	term_wipe_svgalib(x, y, n);

	/* Draw the coloured text */
	gl_colorfont(8, 12, COLOR_OFFSET + (a & 0x0F), font);
	gl_writen(x * CHAR_W, y * CHAR_H, n, (char *) s);
	return (0);
}

/*
 * Low-level graphics routine (assumes valid input)
 * Draw n chars at location (x,y) with value s and attribute a
 */

#ifdef USE_GRAPHICS

# ifdef USE_TRANSPARENCY
static errr term_pict_svgalib(int x, int y, int n,
                              const byte *ap, const char *cp, const byte *tap, const char *tcp)
# else /* USE_TRANSPARENCY */
static errr term_pict_svgalib(int x, int y, int n,
                              const byte *ap, const char *cp)
# endif  /* USE_TRANSPARENCY */
{
	int i;
	int x2, y2;


# ifdef USE_TRANSPARENCY
	/* Hack - Ignore unused transparency data for now */
	(void) tap;
	(void) tcp;
# endif  /* USE_TRANSPARENCY */

	for (i = 0; i < n; i++)
	{
		x2 = (cp[i] & 0x7F) * CHAR_W;
		y2 = (ap[i] & 0x7F) * CHAR_H;

		gl_copyboxfromcontext(buffer, x2, y2, CHAR_W, CHAR_H,
		                      (x + i) * CHAR_W, y * CHAR_H);
	}
	return (0);
}

static void term_load_bitmap(void)
{
	char path[1024];

	byte *temp = NULL;

	int bw, bh;

	/* Build the "graf" path */
	path_build(path, 1024, ANGBAND_DIR_XTRA, "graf");

	sprintf (path, "%s/8x13.bmp", path);

	/* See if the file exists */
	if (fd_close(fd_open(path, O_RDONLY)))
	{
		printf ("Unable to load bitmap data file %s, bailing out....\n", path);
		exit ( -1);
	}

	temp = ReadBMP(path, &bw, &bh);

	/* Blit bitmap into buffer */
	gl_putbox(0, 0, bw, bh, temp);

	FREE(temp, byte);

	return;
}

#endif /* USE_GRAPHICS */

/*
 * Term hook
 * Initialize a new term
 */
static void term_init_svgalib(term *t)
{
	int vgamode;

	/* Only one term */
	(void) t;

	vga_init();

	/* The palette is 256x3 bytes big (RGB). */
	C_MAKE(pal, 768, byte);

#ifdef USE_GRAPHICS

	/* Hardwire this mode in for now */
	vgamode = G1024x768x256;

	/* Set up the bitmap buffer context */
	gl_setcontextvgavirtual(vgamode);
	buffer = gl_allocatecontext();
	gl_getcontext(buffer);

	/* Load bitmap into virtual screen */
	term_load_bitmap();

#endif /* USE_GRAPHICS */

	/* Hardwire this mode in for now */
	vgamode = G640x480x256;

	/* Set up the physical screen context */
	if (vga_setmode(vgamode) < 0)
	{
		quit("Graphics mode not available!");
	}

	gl_setcontextvga(vgamode);
	screen = gl_allocatecontext();
	gl_getcontext(screen);

	/* Is this needed? */
	gl_enablepageflipping(screen);

	/* Set up palette colors */
	setpal();

	/* Load the character font data */
	initfont();

	/* Color 0 isn't transparent */
	gl_setwritemode(WRITEMODE_OVERWRITE);
}

/*
 * Term hook
 * Nuke an old term
 */
static void term_nuke_svgalib(term *t)
{
	/* Only one term */
	(void) t;

	vga_setmode(TEXT);
}

/*
 * Hook SVGAlib routines into term.c
 */
errr init_lsl(void)
{
	term *t = &term_screen_body;

#ifdef USE_GRAPHICS

	if (arg_graphics)
	{
		use_graphics = TRUE;
	}

#endif /* USE_GRAPHICS */

	/* Initialize the term */
	term_init(t, 80, 24, 1024);

	/* The cursor is done via software and needs erasing */
	t->soft_cursor = TRUE;

	t->attr_blank = TERM_DARK;
	t->char_blank = ' ';

	/* Add hooks */
	t->init_hook = term_init_svgalib;
	t->nuke_hook = term_nuke_svgalib;
	t->text_hook = term_text_svgalib;

#ifdef USE_GRAPHICS

	if (use_graphics)
	{
		t->pict_hook = term_pict_svgalib;
		t->higher_pict = TRUE;
	}

#endif /* USE_GRAPHICS */

	t->wipe_hook = term_wipe_svgalib;
	t->curs_hook = term_curs_svgalib;
	t->xtra_hook = term_xtra_svgalib;

	/* Save the term */
	term_screen = t;

	/* Activate it */
	Term_activate(term_screen);

	return (0);
}

#endif /* USE_LSL */

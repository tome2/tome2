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

#include "angband.h"


/*
 * Activate variant-specific features
 *
 * Angband 2.9.3 and close variants don't require any.
 *
 * Angband 2.9.4 alpha and later removed the short-lived
 * can_save flag, so please #define can_save TRUE, or remove
 * all the references to it. They also changed long-lived
 * z-virt macro names. Find C_FREE/C_KILL and replace them
 * with FREE/KILL, which takes one pointer parameter.
 *
 * ZAngband has its own enhanced main-gtk.c as mentioned above, and
 * you *should* use it :-)
 *
 */

#define USE_DOUBLE_TILES	/* Mogami's bigtile patch */


#ifdef USE_GTK2

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

/* /me pffts Solaris */
#ifndef NAME_MAX
#define	NAME_MAX	_POSIX_NAME_MAX
#endif


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

#ifdef USE_GRAPHICS

/*
 * Since GdkRGB doesn't provide us some useful functions...
 */
typedef struct GdkRGBImage GdkRGBImage;

struct GdkRGBImage
{
	gint width;
	gint height;
	gint ref_count;
	guchar *image;
};

#endif /* USE_GRAPHICS */


/*
 * This structure holds everything you need to manipulate terminals
 */
typedef struct term_data term_data;

struct term_data
{
	term t;

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

#ifdef USE_GRAPHICS

	int tile_wid;
	int tile_hgt;

	GdkRGBImage *tiles;
	guint32 bg_pixel;
	GdkRGBImage *trans_buf;

#endif /* USE_GRAPHICS */

	cptr name;
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
static cptr get_default_font(int term)
{
	char buf[64];
	cptr font_name;

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


/*
 * The standard game uses this to implement lighting effects
 * for 16x16 tiles in cave.c...
 *
 * Because of the way it is implemented in X11 ports,
 * we can set this to TRUE even if we are using the 8x8 tileset.
 */
static bool_ use_transparency = TRUE;




/**** Low level routines - memory allocation ****/

/*
 * Hook to "release" memory
 */
static vptr hook_rnfree(vptr v, huge size)
{
	/* Dispose */
	g_free(v);

	/* Success */
	return (NULL);
}


/*
 * Hook to "allocate" memory
 */
static vptr hook_ralloc(huge size)
{
	/* Make a new pointer */
	return (g_malloc(size));
}



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


#ifdef USE_GRAPHICS

/*
 * Graphics mode selector - current setting and requested value
 */
#define GRAF_MODE_NONE	0
#define GRAF_MODE_OLD	1
#define GRAF_MODE_NEW	2

static int graf_mode = GRAF_MODE_NONE;
static int graf_mode_request = GRAF_MODE_NONE;

/*
 * Use smooth rescaling?
 */
static bool_ smooth_rescaling = TRUE;
static bool_ smooth_rescaling_request = TRUE;

/*
 * Dithering
 */
static GdkRgbDither dith_mode = GDK_RGB_DITHER_NORMAL;

/*
 * Need to reload and resize tiles when fonts are changed.
 */
static bool_ resize_request = FALSE;

/*
 * Numbers of columns and rows in current tileset
 * calculated and set by the tile loading code in graf_init()
 * and used by Term_pict_gtk()
 */
static int tile_rows;
static int tile_cols;


/*
 * Directory name(s)
 */
static cptr ANGBAND_DIR_XTRA_GRAF;


/*
 * Be nice to old graphics hardwares -- using GdkRGB.
 *
 * We don't have colour allocation failure any longer this way,
 * even with 8bpp X servers. Gimp *does* work with 8bpp, why not Angband?
 *
 * Initialisation (before any widgets are created)
 *	gdk_rgb_init();
 *	gtk_widget_set_default_colormap (gdk_rgb_get_cmap());
 *	gtk_widget_set_default_visual (gdk_rgb_get_visual());
 *
 * Setting fg/bg colours
 *	void gdk_rgb_gc_set_foreground(GdkGC *gc, guint32 rgb);
 *	void gdk_rgb_gc_set_background(GdkGC *gc, guint32 rgb);
 * where rgb is 0xRRGGBB.
 *
 * Drawing rgb images
 *	void gdk_draw_rgb_image(
 *		GdkDrawable *drawable,
 *		GdkGC *gc,
 *		gint x, gint y,
 *		gint width, gint height,
 *		GdkRgbDither dith,
 *		guchar *rgb_buf,
 *		gint rowstride);
 *
 * dith:
 *	GDK_RGB_DITHER_NORMAL : dither if 8bpp or below
 *	GDK_RGB_DITHER_MAX : dither if 16bpp or below.
 *
 * for 0 <= i < width and 0 <= j < height,
 * the pixel (x + i, y + j) is colored with
 *  red value rgb_buf[j * rowstride + i * 3],
 *  green value rgb_buf[j * rowstride + i * 3 + 1], and
 *  blue value rgb_buf[j * rowstride + i * 3 + 2].
 */

/*
 * gdk_image compatibility functions - should be part of gdk, IMHO.
 */

/*
 * Create GdkRGBImage of width * height and return pointer
 * to it. Returns NULL on failure
 */
static GdkRGBImage *gdk_rgb_image_new(
        gint width,
        gint height)
{
	GdkRGBImage *result;

	/* Allocate a struct */
	result = g_new(GdkRGBImage, 1);

	/* Oops */
	if (result == NULL) return (NULL);

	/* Allocate buffer */
	result->image = g_new0(guchar, width * height * 3);

	/* Oops */
	if (result->image == NULL)
	{
		g_free(result);
		return (NULL);
	}

	/* Initialise size fields */
	result->width = width;
	result->height = height;

	/* Initialise reference count */
	result->ref_count = 1;

	/* Success */
	return (result);
}

/*
 * Free a GdkRGBImage
 */
static void gdk_rgb_image_destroy(
        GdkRGBImage *im)
{
	/* Paranoia */
	if (im == NULL) return;

	/* Free the RGB buffer */
	g_free(im->image);

	/* Free the structure */
	g_free(im);
}


/*
 * Write RGB pixel of the format 0xRRGGBB to (x, y) in GdkRGBImage
 */
static void gdk_rgb_image_put_pixel(
        GdkRGBImage *im,
        gint x,
        gint y,
        guint32 pixel)
{
	guchar *rgbp;

	/* Paranoia */
	g_return_if_fail(im != NULL);

	/* Paranoia */
	if ((x < 0) || (x >= im->width)) return;

	/* Paranoia */
	if ((y < 0) || (y >= im->height)) return;

	/* Access RGB data */
	rgbp = &im->image[(y * im->width * 3) + (x * 3)];

	/* Red */
	*rgbp++ = (pixel >> 16) & 0xFF;
	/* Green */
	*rgbp++ = (pixel >> 8) & 0xFF;
	/* Blue */
	*rgbp = pixel & 0xFF;
}


/*
 * Returns RGB pixel (0xRRGGBB) at (x, y) in GdkRGBImage
 */
static guint32 gdk_rgb_image_get_pixel(
        GdkRGBImage *im,
        gint x,
        gint y)
{
	guchar *rgbp;

	/* Paranoia */
	if (im == NULL) return (0);

	/* Paranoia - returns black */
	if ((x < 0) || (x >= im->width)) return (0);

	/* Paranoia */
	if ((y < 0) || (y >= im->height)) return (0);

	/* Access RGB data */
	rgbp = &im->image[(y * im->width * 3) + (x * 3)];

	/* Return result */
	return ((rgbp[0] << 16) | (rgbp[1] << 8) | (rgbp[2]));
}


/*
 * Since gdk_draw_rgb_image is a bit harder to use than it's
 * GdkImage counterpart, I wrote a grue function that takes
 * exactly the same parameters as gdk_draw_image, with
 * the GdkImage parameter replaced with GdkRGBImage.
 */
static void gdk_draw_rgb_image_2(
        GdkDrawable *drawable,
        GdkGC *gc,
        GdkRGBImage *image,
        gint xsrc,
        gint ysrc,
        gint xdest,
        gint ydest,
        gint width,
        gint height)
{
	/* Paranoia */
	g_return_if_fail(drawable != NULL);
	g_return_if_fail(image != NULL);

	/* Paranoia */
	if (xsrc < 0 || (xsrc + width - 1) >= image->width) return;
	if (ysrc < 0 || (ysrc + height - 1) >= image->height) return;

	/* Draw the image at (xdest, ydest), with dithering if bpp <= 8/16 */
	gdk_draw_rgb_image(
	        drawable,
	        gc,
	        xdest,
	        ydest,
	        width,
	        height,
	        dith_mode,
	        &image->image[(ysrc * image->width * 3) + (xsrc * 3)],
	        image->width * 3);
}


/*
 * Code for smooth icon rescaling from Uwe Siems, Jan 2000
 *
 * XXX XXX Duplication of maid-x11.c, again. It doesn't do any colour
 * allocation, either.
 */

/*
 * to save ourselves some labour, define a maximum expected icon width here:
 */
#define MAX_ICON_WIDTH 32


/*
 * Each pixel is kept in this structure during smooth rescaling
 * calculations, to make things a bit easier
 */
typedef struct rgb_type rgb_type;

struct rgb_type
{
	guint32 red;
	guint32 green;
	guint32 blue;
};

/*
 * Because there are many occurences of this, and because
 * it's logical to do so...
 */
#define pixel_to_rgb(pix, rgb_buf) \
(rgb_buf)->red   = ((pix) >> 16) & 0xFF; \
(rgb_buf)->green = ((pix) >> 8)  & 0xFF; \
(rgb_buf)->blue  = (pix) & 0xFF


/*
 * get_scaled_row reads a scan from the given GdkRGBImage, scales it smoothly
 * and returns the red, green and blue values in arrays.
 * The values in this arrays must be divided by a certain value that is
 * calculated in scale_icon.
 * x, y is the position, iw is the input width and ow the output width
 * scan must be sufficiently sized
 */
static void get_scaled_row(
        GdkRGBImage *im,
        int x,
        int y,
        int iw,
        int ow,
        rgb_type *scan)
{
	int xi, si, sifrac, ci, cifrac, add_whole, add_frac;
	guint32 pix;
	rgb_type prev;
	rgb_type next;
	bool_ get_next_pix;

	/* Unscaled */
	if (iw == ow)
	{
		for (xi = 0; xi < ow; xi++)
		{
			pix = gdk_rgb_image_get_pixel(im, x + xi, y);
			pixel_to_rgb(pix, &scan[xi]);
		}
	}

	/* Scaling by subsampling (grow) */
	else if (iw < ow)
	{
		iw--;
		ow--;

		/* read first pixel: */
		pix = gdk_rgb_image_get_pixel(im, x, y);
		pixel_to_rgb(pix, &next);
		prev = next;

		/* si and sifrac give the subsampling position: */
		si = x;
		sifrac = 0;

		/* get_next_pix tells us, that we need the next pixel */
		get_next_pix = TRUE;

		for (xi = 0; xi <= ow; xi++)
		{
			if (get_next_pix)
			{
				prev = next;
				if (xi < ow)
				{
					/* only get next pixel if in same icon */
					pix = gdk_rgb_image_get_pixel(im, si + 1, y);
					pixel_to_rgb(pix, &next);
				}
			}

			/* calculate subsampled color values: */
			/* division by ow occurs in scale_icon */
			scan[xi].red = prev.red * (ow - sifrac) + next.red * sifrac;
			scan[xi].green = prev.green * (ow - sifrac) + next.green * sifrac;
			scan[xi].blue = prev.blue * (ow - sifrac) + next.blue * sifrac;

			/* advance sampling position: */
			sifrac += iw;
			if (sifrac >= ow)
			{
				si++;
				sifrac -= ow;
				get_next_pix = TRUE;
			}
			else
			{
				get_next_pix = FALSE;
			}

		}
	}

	/* Scaling by averaging (shrink) */
	else
	{
		/* width of an output pixel in input pixels: */
		add_whole = iw / ow;
		add_frac = iw % ow;

		/* start position of the first output pixel: */
		si = x;
		sifrac = 0;

		/* get first input pixel: */
		pix = gdk_rgb_image_get_pixel(im, x, y);
		pixel_to_rgb(pix, &next);

		for (xi = 0; xi < ow; xi++)
		{
			/* find endpoint of the current output pixel: */
			ci = si + add_whole;
			cifrac = sifrac + add_frac;
			if (cifrac >= ow)
			{
				ci++;
				cifrac -= ow;
			}

			/* take fraction of current input pixel (starting segment): */
			scan[xi].red = next.red * (ow - sifrac);
			scan[xi].green = next.green * (ow - sifrac);
			scan[xi].blue = next.blue * (ow - sifrac);
			si++;

			/* add values for whole pixels: */
			while (si < ci)
			{
				rgb_type tmp_rgb;

				pix = gdk_rgb_image_get_pixel(im, si, y);
				pixel_to_rgb(pix, &tmp_rgb);
				scan[xi].red += tmp_rgb.red * ow;
				scan[xi].green += tmp_rgb.green * ow;
				scan[xi].blue += tmp_rgb.blue * ow;
				si++;
			}

			/* add fraction of current input pixel (ending segment): */
			if (xi < ow - 1)
			{
				/* only get next pixel if still in icon: */
				pix = gdk_rgb_image_get_pixel(im, si, y);
				pixel_to_rgb(pix, &next);
			}

			sifrac = cifrac;
			if (sifrac > 0)
			{
				scan[xi].red += next.red * sifrac;
				scan[xi].green += next.green * sifrac;
				scan[xi].blue += next.blue * sifrac;
			}
		}
	}
}


/*
 * put_rgb_scan takes arrays for red, green and blue and writes pixel values
 * according to this values in the GdkRGBImage-structure. w is the number of
 * pixels to write and div is the value by which all red/green/blue values
 * are divided first.
 */
static void put_rgb_scan(
        GdkRGBImage *im,
        int x,
        int y,
        int w,
        int div,
        rgb_type *scan)
{
	int xi;
	guint32 pix;
	guint32 adj = div / 2;

	for (xi = 0; xi < w; xi++)
	{
		byte r, g, b;

		/* un-factor the RGB values */
		r = (scan[xi].red + adj) / div;
		g = (scan[xi].green + adj) / div;
		b = (scan[xi].blue + adj) / div;

		/* Make a (virtual) 24-bit pixel */
		pix = (r << 16) | (g << 8) | (b);

		/* Draw it into image */
		gdk_rgb_image_put_pixel(im, x + xi, y, pix);
	}
}


/*
 * scale_icon transfers an area from GdkRGBImage im_in, locate (x1,y1) to
 * im_out, locate (x2, y2). Source size is (ix, iy) and destination size
 * is (ox, oy).
 *
 * It does this by getting icon scan line from get_scaled_scan and handling
 * them the same way as pixels are handled in get_scaled_scan.
 * This even allows icons to be scaled differently in horizontal and
 * vertical directions (eg. shrink horizontal, grow vertical).
 */
static void scale_icon(
        GdkRGBImage *im_in,
        GdkRGBImage *im_out,
        int x1,
        int y1,
        int x2,
        int y2,
        int ix,
        int iy,
        int ox,
        int oy)
{
	int div;
	int xi, yi, si, sifrac, ci, cifrac, add_whole, add_frac;

	/* buffers for pixel rows: */
	rgb_type prev[MAX_ICON_WIDTH];
	rgb_type next[MAX_ICON_WIDTH];
	rgb_type temp[MAX_ICON_WIDTH];

	bool_ get_next_row;

	/* get divider value for the horizontal scaling: */
	if (ix == ox)
		div = 1;
	else if (ix < ox)
		div = ox - 1;
	else
		div = ix;

	/* no scaling needed vertically: */
	if (iy == oy)
	{
		for (yi = 0; yi < oy; yi++)
		{
			get_scaled_row(im_in, x1, y1 + yi, ix, ox, temp);
			put_rgb_scan(im_out, x2, y2 + yi, ox, div, temp);
		}
	}

	/* scaling by subsampling (grow): */
	else if (iy < oy)
	{
		iy--;
		oy--;
		div *= oy;

		/* get first row: */
		get_scaled_row(im_in, x1, y1, ix, ox, next);

		/* si and sifrac give the subsampling position: */
		si = y1;
		sifrac = 0;

		/* get_next_row tells us, that we need the next row */
		get_next_row = TRUE;
		for (yi = 0; yi <= oy; yi++)
		{
			if (get_next_row)
			{
				for (xi = 0; xi < ox; xi++)
				{
					prev[xi] = next[xi];
				}
				if (yi < oy)
				{
					/* only get next row if in same icon */
					get_scaled_row(im_in, x1, si + 1, ix, ox, next);
				}
			}

			/* calculate subsampled color values: */
			/* division by oy occurs in put_rgb_scan */
			for (xi = 0; xi < ox; xi++)
			{
				temp[xi].red = (prev[xi].red * (oy - sifrac) +
				                next[xi].red * sifrac);
				temp[xi].green = (prev[xi].green * (oy - sifrac) +
				                  next[xi].green * sifrac);
				temp[xi].blue = (prev[xi].blue * (oy - sifrac) +
				                 next[xi].blue * sifrac);
			}

			/* write row to output image: */
			put_rgb_scan(im_out, x2, y2 + yi, ox, div, temp);

			/* advance sampling position: */
			sifrac += iy;
			if (sifrac >= oy)
			{
				si++;
				sifrac -= oy;
				get_next_row = TRUE;
			}
			else
			{
				get_next_row = FALSE;
			}

		}
	}

	/* scaling by averaging (shrink) */
	else
	{
		div *= iy;

		/* height of a output row in input rows: */
		add_whole = iy / oy;
		add_frac = iy % oy;

		/* start position of the first output row: */
		si = y1;
		sifrac = 0;

		/* get first input row: */
		get_scaled_row(im_in, x1, y1, ix, ox, next);
		for (yi = 0; yi < oy; yi++)
		{
			/* find endpoint of the current output row: */
			ci = si + add_whole;
			cifrac = sifrac + add_frac;
			if (cifrac >= oy)
			{
				ci++;
				cifrac -= oy;
			}

			/* take fraction of current input row (starting segment): */
			for (xi = 0; xi < ox; xi++)
			{
				temp[xi].red = next[xi].red * (oy - sifrac);
				temp[xi].green = next[xi].green * (oy - sifrac);
				temp[xi].blue = next[xi].blue * (oy - sifrac);
			}
			si++;

			/* add values for whole pixels: */
			while (si < ci)
			{
				get_scaled_row(im_in, x1, si, ix, ox, next);
				for (xi = 0; xi < ox; xi++)
				{
					temp[xi].red += next[xi].red * oy;
					temp[xi].green += next[xi].green * oy;
					temp[xi].blue += next[xi].blue * oy;
				}
				si++;
			}

			/* add fraction of current input row (ending segment): */
			if (yi < oy - 1)
			{
				/* only get next row if still in icon: */
				get_scaled_row(im_in, x1, si, ix, ox, next);
			}
			sifrac = cifrac;
			for (xi = 0; xi < ox; xi++)
			{
				temp[xi].red += next[xi].red * sifrac;
				temp[xi].green += next[xi].green * sifrac;
				temp[xi].blue += next[xi].blue * sifrac;
			}

			/* write row to output image: */
			put_rgb_scan(im_out, x2, y2 + yi, ox, div, temp);
		}
	}
}


/*
 * Rescale icons using sort of anti-aliasing technique.
 */
static GdkRGBImage *resize_tiles_smooth(
        GdkRGBImage *im,
        int ix,
        int iy,
        int ox,
        int oy)
{
	int width1, height1, width2, height2;
	int x1, x2, y1, y2;

	GdkRGBImage *tmp;

	/* Original size */
	width1 = im->width;
	height1 = im->height;

	/* Rescaled size */
	width2 = ox * width1 / ix;
	height2 = oy * height1 / iy;

	/* Allocate GdkRGBImage for resized tiles */
	tmp = gdk_rgb_image_new(width2, height2);

	/* Oops */
	if (tmp == NULL) return (NULL);

	/* Scale each icon */
	for (y1 = 0, y2 = 0; (y1 < height1) && (y2 < height2); y1 += iy, y2 += oy)
	{
		for (x1 = 0, x2 = 0; (x1 < width1) && (x2 < width2); x1 += ix, x2 += ox)
		{
			scale_icon(im, tmp, x1, y1, x2, y2,
			           ix, iy, ox, oy);
		}
	}

	return tmp;
}


/*
 * Steven Fuerst's tile resizing code
 * Taken from Z because I think the algorithm is cool.
 */

/* 24-bit version - GdkRGB uses 24 bit RGB data internally */
static void copy_pixels(
        int wid,
        int y,
        int offset,
        int *xoffsets,
        GdkRGBImage *old_image,
        GdkRGBImage *new_image)
{
	int i;

	/* Get source and destination */
	byte *src = &old_image->image[offset * old_image->width * 3];
	byte *dst = &new_image->image[y * new_image->width * 3];

	/* Copy to the image */
	for (i = 0; i < wid; i++)
	{
		*dst++ = src[3 * xoffsets[i]];
		*dst++ = src[3 * xoffsets[i] + 1];
		*dst++ = src[3 * xoffsets[i] + 2];
	}
}


/*
 * Resize ix * iy pixel tiles in old to ox * oy pixels
 * and return a new GdkRGBImage containing the resized tiles
 */
static GdkRGBImage *resize_tiles_fast(
        GdkRGBImage *old_image,
        int ix,
        int iy,
        int ox,
        int oy)
{
	GdkRGBImage *new_image;

	int old_wid, old_hgt;

	int new_wid, new_hgt;

	int add, remainder, rem_tot, offset;

	int *xoffsets;

	int i;


	/* Get the size of the old image */
	old_wid = old_image->width;
	old_hgt = old_image->height;

	/* Calculate the size of the new image */
	new_wid = (old_wid / ix) * ox;
	new_hgt = (old_hgt / iy) * oy;

	/* Allocate a GdkRGBImage to store resized tiles */
	new_image = gdk_rgb_image_new(new_wid, new_hgt);

	/* Paranoia */
	if (new_image == NULL) return (NULL);

	/* now begins the cool part of SF's code */

	/*
	 * Calculate an offsets table, so the transformation
	 * is faster.  This is much like the Bresenham algorithm
	 */

	/* Set up x offset table */
	C_MAKE(xoffsets, new_wid, int);

	/* Initialize line parameters */
	add = old_wid / new_wid;
	remainder = old_wid % new_wid;

	/* Start at left */
	offset = 0;

	/* Half-tile offset so 'line' is centered correctly */
	rem_tot = new_wid / 2;

	for (i = 0; i < new_wid; i++)
	{
		/* Store into the table */
		xoffsets[i] = offset;

		/* Move to next entry */
		offset += add;

		/* Take care of fractional part */
		rem_tot += remainder;
		if (rem_tot >= new_wid)
		{
			rem_tot -= new_wid;
			offset++;
		}
	}

	/* Scan each row */

	/* Initialize line parameters */
	add = old_hgt / new_hgt;
	remainder = old_hgt % new_hgt;

	/* Start at left */
	offset = 0;

	/* Half-tile offset so 'line' is centered correctly */
	rem_tot = new_hgt / 2;

	for (i = 0; i < new_hgt; i++)
	{
		/* Copy pixels to new image */
		copy_pixels(new_wid, i, offset, xoffsets, old_image, new_image);

		/* Move to next entry */
		offset += add;

		/* Take care of fractional part */
		rem_tot += remainder;
		if (rem_tot >= new_hgt)
		{
			rem_tot -= new_hgt;
			offset++;
		}
	}

	/* Free offset table */
	C_FREE(xoffsets, new_wid, int);

	return (new_image);
}


/*
 * Resize an image of ix * iy pixels and return a newly allocated
 * image of ox * oy pixels.
 */
static GdkRGBImage *resize_tiles(
        GdkRGBImage *im,
        int ix,
        int iy,
        int ox,
        int oy)
{
	GdkRGBImage *result;

	/*
	 * I hope we can always use this with GdkRGB, which uses a 5x5x5
	 * colour cube (125 colours) by default, and resort to dithering
	 * when it can't find good match there or expand the cube, so it
	 * works with 8bpp X servers.
	 */
	if (smooth_rescaling_request && (ix != ox || iy != oy))
	{
		result = resize_tiles_smooth(im, ix, iy, ox, oy);
	}

	/*
	 * Unless smoothing is requested by user, we use the fast
	 * resizing code.
	 */
	else
	{
		result = resize_tiles_fast(im, ix, iy, ox, oy);
	}

	/* Return rescaled tiles, or NULL */
	return (result);
}


/*
 * Tile loaders - XPM and BMP
 */

/*
 * A helper function for the XPM loader
 *
 * Read next string delimited by double quotes from
 * the input stream. Return TRUE on success, FALSE
 * if it finds EOF or buffer overflow.
 *
 * I never mean this to be generic, so its EOF and buffer
 * overflow behaviour is terribly stupid -- there are no
 * provisions for recovery.
 *
 * CAVEAT: treatment of backslash is not compatible with the standard
 * C usage XXX XXX XXX XXX
 */
static bool_ read_str(char *buf, u32b len, FILE *f)
{
	int c;

	/* Paranoia - Buffer too small */
	if (len <= 0) return (FALSE);

	/* Find " */
	while ((c = getc(f)) != '"')
	{
		/* Premature EOF */
		if (c == EOF) return (FALSE);
	}

	while (1)
	{
		/* Read next char */
		c = getc(f);

		/* Premature EOF */
		if (c == EOF) return (FALSE);

		/* Terminating " */
		if (c == '"') break;

		/* Escape */
		if (c == '\\')
		{
			/* Use next char */
			c = getc(f);

			/* Premature EOF */
			if (c == EOF) return (FALSE);
		}

		/* Store character in the buffer */
		*buf++ = c;

		/* Decrement count */
		len--;

		/* Buffer full - we have to place a NULL at the end */
		if (len <= 0) return (FALSE);
	}

	/* Make a C string if there's room left */
	if (len > 0) *buf = '\0';

	/* Success */
	return (TRUE);
}


/*
 * Remember pixel symbol to RGB colour mappings
 */

/*
 * I've forgot the formula, but I remember prime number yields
 * good results
 */
#define HASH_SIZE 19

typedef struct pal_type pal_type;

struct pal_type
{
	u32b str;
	u32b rgb;
	pal_type *next;
};


/*
 * A simple, slow and stupid XPM loader
 */
static GdkRGBImage *load_xpm(cptr filename)
{
	FILE *f;
	GdkRGBImage *img = NULL;
	int width, height, colours, chars;
	int i, j, k;
	bool_ ret;
	pal_type *pal = NULL;
	pal_type *head[HASH_SIZE];
	u32b buflen = 0;
	char *lin = NULL;
	char buf[1024];

	/* Build path to the XPM file */
	path_build(buf, 1024, ANGBAND_DIR_XTRA_GRAF, filename);

	/* Open it */
	f = my_fopen(buf, "r");

	/* Oops */
	if (f == NULL) return (NULL);

	/* Read header */
	ret = read_str(buf, 1024, f);

	/* Oops */
	if (!ret)
	{
		/* Notify error */
		plog("Cannot find XPM header");

		/* Failure */
		goto oops;
	}

	/* Parse header */
	if (4 != sscanf(buf, "%d %d %d %d", &width, &height, &colours, &chars))
	{
		/* Notify error */
		plog("Bad XPM header");

		/* Failure */
		goto oops;
	}

	/*
	 * Paranoia - the code can handle upto four letters per pixel,
	 * but such large number of colours certainly requires a smarter
	 * symbol-to-colour mapping algorithm...
	 */
	if ((width <= 0) || (height <= 0) || (colours <= 0) || (chars <= 0) ||
	                (chars > 2))
	{
		/* Notify error */
		plog("Invalid width/height/depth");

		/* Failure */
		goto oops;
	}

	/* Allocate palette */
	C_MAKE(pal, colours, pal_type);

	/* Initialise hash table */
	for (i = 0; i < HASH_SIZE; i++) head[i] = NULL;

	/* Parse palette */
	for (i = 0; i < colours; i++)
	{
		u32b tmp;
		int h_idx;

		/* Read next string */
		ret = read_str(buf, 1024, f);

		/* Check I/O result */
		if (!ret)
		{
			/* Notify error */
			plog("EOF in palette");

			/* Failure */
			goto oops;
		}

		/* Clear symbol code */
		tmp = 0;

		/* Encode pixel symbol */
		for (j = 0; j < chars; j++)
		{
			tmp = (tmp << 8) | (buf[j] & 0xFF);
		}

		/* Remember it */
		pal[i].str = tmp;

		/* Skip spaces */
		while ((buf[j] == ' ') || (buf[j] == '\t')) j++;

		/* Verify 'c' */
		if (buf[j] != 'c')
		{
			/* Notify error */
			plog("No 'c' in palette definition");

			/* Failure */
			goto oops;
		}

		/* Advance cursor */
		j++;

		/* Skip spaces */
		while ((buf[j] == ' ') || (buf[j] == '\t')) j++;

		/* Hack - Assume 'None' */
		if (buf[j] == 'N')
		{
			/* Angband always uses black background */
			pal[i].rgb = 0x000000;
		}

		/* Read colour */
		else if ((1 != sscanf(&buf[j], "#%06lX", &tmp)) &&
		                (1 != sscanf(&buf[j], "#%06lx", &tmp)))
		{
			/* Notify error */
			plog("Badly formatted colour");

			/* Failure */
			goto oops;
		}

		/* Remember it */
		pal[i].rgb = tmp;

		/* Store it in hash table as well */
		h_idx = pal[i].str % HASH_SIZE;

		/* Link the entry */
		pal[i].next = head[h_idx];
		head[h_idx] = &pal[i];
	}

	/* Allocate image */
	img = gdk_rgb_image_new(width, height);

	/* Oops */
	if (img == NULL)
	{
		/* Notify error */
		plog("Cannot allocate image");

		/* Failure */
		goto oops;
	}

	/* Calculate buffer length */
	buflen = width * chars + 1;

	/* Allocate line buffer */
	C_MAKE(lin, buflen, char);

	/* For each row */
	for (i = 0; i < height; i++)
	{
		/* Read a row of image data */
		ret = read_str(lin, buflen, f);

		/* Oops */
		if (!ret)
		{
			/* Notify error */
			plog("EOF in middle of image data");

			/* Failure */
			goto oops;
		}

		/* For each column */
		for (j = 0; j < width; j++)
		{
			u32b tmp;
			pal_type *h_ptr;

			/* Clear encoded pixel */
			tmp = 0;

			/* Encode pixel symbol */
			for (k = 0; k < chars; k++)
			{
				tmp = (tmp << 8) | (lin[j * chars + k] & 0xFF);
			}

			/* Find colour */
			for (h_ptr = head[tmp % HASH_SIZE];
			                h_ptr != NULL;
			                h_ptr = h_ptr->next)
			{
				/* Found a match */
				if (h_ptr->str == tmp) break;
			}

			/* No match found */
			if (h_ptr == NULL)
			{
				/* Notify error */
				plog("Invalid pixel symbol");

				/* Failure */
				goto oops;
			}

			/* Draw it */
			gdk_rgb_image_put_pixel(
			        img,
			        j,
			        i,
			        h_ptr->rgb);
		}
	}

	/* Close file */
	my_fclose(f);

	/* Free line buffer */
	C_FREE(lin, buflen, char);

	/* Free palette */
	C_FREE(pal, colours, pal_type);

	/* Return result */
	return (img);

oops:

	/* Close file */
	my_fclose(f);

	/* Free image */
	if (img) gdk_rgb_image_destroy(img);

	/* Free line buffer */
	if (lin) C_FREE(lin, buflen, char);

	/* Free palette */
	if (pal) C_FREE(pal, colours, pal_type);

	/* Failure */
	return (NULL);
}


/*
 * A BMP loader, yet another duplication of maid-x11.c functions.
 *
 * Another duplication, again because of different image format and
 * avoidance of colour allocation.
 *
 * XXX XXX XXX XXX Should avoid using a propriatary and closed format.
 * Since it's much bigger than gif that was used before, why don't
 * we switch to XPM?  NetHack does.  Well, NH has always been much
 * closer to the GNU/Un*x camp and it's GPL'ed quite early...
 *
 * The names and naming convention are worse than the worst I've ever
 * seen, so I deliberately changed them to fit well with the rest of
 * the code. Or are they what xx calls them? If it's the case, there's
 * no reason to follow *their* words.
 */

/*
 * BMP file header
 */
typedef struct bmp_file_type bmp_file_type;

struct bmp_file_type
{
	u16b type;
	u32b size;
	u16b reserved1;
	u16b reserved2;
	u32b offset;
};


/*
 * BMP file information fields
 */
typedef struct bmp_info_type bmp_info_type;

struct bmp_info_type
{
	u32b size;
	u32b width;
	u32b height;
	u16b planes;
	u16b bit_count;
	u32b compression;
	u32b size_image;
	u32b x_pels_per_meter;
	u32b y_pels_per_meter;
	u32b colors_used;
	u32b color_importand;
};

/*
 * "RGBQUAD" type.
 */
typedef struct rgb_quad_type rgb_quad_type;

struct rgb_quad_type
{
	unsigned char b, g, r;
	unsigned char filler;
};


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
 * Read a BMP file (a certain trademark nuked)
 *
 * This function replaces the old ReadRaw and RemapColors functions.
 *
 * Assumes that the bitmap has a size such that no padding is needed in
 * various places.  Currently only handles bitmaps with 3 to 256 colors.
 */
GdkRGBImage *load_bmp(cptr filename)
{
	FILE *f;

	char path[1024];

	bmp_file_type file_hdr;
	bmp_info_type info_hdr;

	GdkRGBImage *result = NULL;

	int ncol;

	int i;

	u32b x, y;

	guint32 colour_pixels[256];


	/* Build the path to the bmp file */
	path_build(path, 1024, ANGBAND_DIR_XTRA_GRAF, filename);

	/* Open the BMP file */
	f = fopen(path, "r");

	/* No such file */
	if (f == NULL)
	{
		return (NULL);
	}

	/* Read the "bmp_file_type" */
	rd_u16b(f, &file_hdr.type);
	rd_u32b(f, &file_hdr.size);
	rd_u16b(f, &file_hdr.reserved1);
	rd_u16b(f, &file_hdr.reserved2);
	rd_u32b(f, &file_hdr.offset);

	/* Read the "bmp_info_type" */
	rd_u32b(f, &info_hdr.size);
	rd_u32b(f, &info_hdr.width);
	rd_u32b(f, &info_hdr.height);
	rd_u16b(f, &info_hdr.planes);
	rd_u16b(f, &info_hdr.bit_count);
	rd_u32b(f, &info_hdr.compression);
	rd_u32b(f, &info_hdr.size_image);
	rd_u32b(f, &info_hdr.x_pels_per_meter);
	rd_u32b(f, &info_hdr.y_pels_per_meter);
	rd_u32b(f, &info_hdr.colors_used);
	rd_u32b(f, &info_hdr.color_importand);

	/* Verify the header */
	if (feof(f) ||
	                (file_hdr.type != 19778) ||
	                (info_hdr.size != 40))
	{
		plog(format("Incorrect BMP file format %s", filename));
		fclose(f);
		return (NULL);
	}

	/*
	 * The two headers above occupy 54 bytes total
	 * The "offset" field says where the data starts
	 * The "colors_used" field does not seem to be reliable
	 */

	/* Compute number of colors recorded */
	ncol = (file_hdr.offset - 54) / 4;

	for (i = 0; i < ncol; i++)
	{
		rgb_quad_type clr;

		/* Read an "rgb_quad_type" */
		rd_byte(f, &clr.b);
		rd_byte(f, &clr.g);
		rd_byte(f, &clr.r);
		rd_byte(f, &clr.filler);

		/* Remember the pixel */
		colour_pixels[i] = (clr.r << 16) | (clr.g << 8) | (clr.b);
	}

	/* Allocate GdkRGBImage large enough to store the image */
	result = gdk_rgb_image_new(info_hdr.width, info_hdr.height);

	/* Failure */
	if (result == NULL)
	{
		fclose(f);
		return (NULL);
	}

	for (y = 0; y < info_hdr.height; y++)
	{
		u32b y2 = info_hdr.height - y - 1;

		for (x = 0; x < info_hdr.width; x++)
		{
			int ch = getc(f);

			/* Verify not at end of file XXX XXX */
			if (feof(f))
			{
				plog(format("Unexpected end of file in %s", filename));
				gdk_rgb_image_destroy(result);
				fclose(f);
				return (NULL);
			}

			if (info_hdr.bit_count == 24)
			{
				int c3, c2 = getc(f);

				/* Verify not at end of file XXX XXX */
				if (feof(f))
				{
					plog(format("Unexpected end of file in %s", filename));
					gdk_rgb_image_destroy(result);
					fclose(f);
					return (NULL);
				}

				c3 = getc(f);

				/* Verify not at end of file XXX XXX */
				if (feof(f))
				{
					plog(format("Unexpected end of file in %s", filename));
					gdk_rgb_image_destroy(result);
					fclose(f);
					return (NULL);
				}

				/* Draw the pixel */
				gdk_rgb_image_put_pixel(
				        result,
				        x,
				        y2,
				        (ch << 16) | (c2 << 8) | (c3));
			}
			else if (info_hdr.bit_count == 8)
			{
				gdk_rgb_image_put_pixel(result, x, y2, colour_pixels[ch]);
			}
			else if (info_hdr.bit_count == 4)
			{
				gdk_rgb_image_put_pixel(result, x, y2, colour_pixels[ch / 16]);
				x++;
				gdk_rgb_image_put_pixel(result, x, y2, colour_pixels[ch % 16]);
			}
			else
			{
				/* Technically 1 bit is legal too */
				plog(format("Illegal bit count %d in %s",
				            info_hdr.bit_count, filename));
				gdk_rgb_image_destroy(result);
				fclose(f);
				return (NULL);
			}
		}
	}

	fclose(f);

	return result;
}


/*
 * Try to load an XPM file, or a BMP file if it fails
 *
 * Choice of file format may better be made yet another option XXX
 */
static GdkRGBImage *load_tiles(cptr basename)
{
	char buf[32];
	GdkRGBImage *img;

	/* build xpm file name */
	strnfmt(buf, 32, "%s.xpm", basename);

	/* Try to load it */
	img = load_xpm(buf);

	/* OK */
	if (img) return (img);

	/* Try again for a bmp file */
	strnfmt(buf, 32, "%s.bmp", basename);

	/* Try loading it */
	img = load_bmp(buf);

	/* Return result, success or failure */
	return (img);
}


/*
 * Free all tiles and graphics buffers associated with windows
 *
 * This is conspirator of graf_init() below, sharing its inefficiency
 */
static void graf_nuke()
{
	int i;

	term_data *td;


	/* Nuke all terms */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Access term_data structure */
		td = &data[i];

		/* Disable graphics */
		td->t.higher_pict = FALSE;

		/* Free previously allocated tiles */
		if (td->tiles) gdk_rgb_image_destroy(td->tiles);

		/* Forget pointer */
		td->tiles = NULL;

		/* Free previously allocated transparency buffer */
		if (td->trans_buf) gdk_rgb_image_destroy(td->trans_buf);

		/* Forget stale pointer */
		td->trans_buf = NULL;
	}
}


/*
 * Load tiles, scale them to current font size, and store a pointer
 * to them in a term_data structure for each term.
 *
 * XXX XXX XXX This is a terribly stupid quick hack.
 *
 * XXX XXX XXX Windows using the same font should share resized tiles
 */
static bool_ graf_init(
        cptr filename,
        int tile_wid,
        int tile_hgt)
{
	term_data *td;

	bool_ result;

	GdkRGBImage *raw_tiles, *scaled_tiles;

	GdkRGBImage *buffer;

	int i;


	/* Paranoia */
	if (filename == NULL) return (FALSE);

	/* Load tiles */
	raw_tiles = load_tiles(filename);

	/* Oops */
	if (raw_tiles == NULL)
	{
		/* Clean up */
		graf_nuke();

		/* Failure */
		return (FALSE);
	}

	/* Calculate and remember numbers of rows and columns */
	tile_rows = raw_tiles->height / tile_hgt;
	tile_cols = raw_tiles->width / tile_wid;

	/* Be optimistic */
	result = TRUE;


	/*
	 * (Re-)init each term
	 * XXX It might help speeding this up to avoid doing so if a window
	 * doesn't need graphics (e.g. inventory/equipment and message recall).
	 */
	for (i = 0; i < MAX_TERM_DATA; i++)
	{
		/* Access term_data */
		td = &data[i];

		/* Shouldn't waste anything for unused terms */
		if (!td->shown) continue;

		/* Enable graphics */
		td->t.higher_pict = TRUE;

		/* See if we need rescaled tiles XXX */
		if ((td->tiles == NULL) ||
		                (td->tiles->width != td->tile_wid * tile_cols) ||
		                (td->tiles->height != td->tile_hgt * tile_rows))
		{
			/* Free old tiles if present */
			if (td->tiles) gdk_rgb_image_destroy(td->tiles);

			/* Forget pointer */
			td->tiles = NULL;

			/* Scale the tiles to current font bounding rect */
			scaled_tiles = resize_tiles(
			                       raw_tiles,
			                       tile_wid, tile_hgt,
			                       td->tile_wid, td->tile_hgt);

			/* Oops */
			if (scaled_tiles == NULL)
			{
				/* Failure */
				result = FALSE;

				break;
			}

			/* Store it */
			td->tiles = scaled_tiles;
		}

		/* See if we have to (re)allocate a new buffer XXX */
		if ((td->trans_buf == NULL) ||
		                (td->trans_buf->width != td->tile_wid) ||
		                (td->trans_buf->height != td->tile_hgt))
		{
			/* Free old buffer if present */
			if (td->trans_buf) gdk_rgb_image_destroy(td->trans_buf);

			/* Forget pointer */
			td->trans_buf = NULL;

			/* Allocate a new buffer */
			buffer = gdk_rgb_image_new(td->tile_wid, td->tile_hgt);

			/* Oops */
			if (buffer == NULL)
			{
				/* Failure */
				result = FALSE;

				break;
			}

			/* Store it */
			td->trans_buf = buffer;
		}

		/*
		 * Giga-Hack - assume top left corner of 0x86/0x80 should be
		 * in the background colour XXX XXX XXX XXX
		 */
		td->bg_pixel = gdk_rgb_image_get_pixel(
		                       raw_tiles,
		                       0,
		                       tile_hgt * 6);

	}


	/* Alas, we need to free wasted images */
	if (result == FALSE) graf_nuke();

	/* We don't need the raw image any longer */
	gdk_rgb_image_destroy(raw_tiles);

	/* Report success or failure */
	return (result);
}


/*
 * React to various changes in graphics mode settings
 *
 * It is *not* a requirement for tiles to have same pixel width and height.
 * The program can work with any conbinations of graf_wid and graf_hgt
 * (oops, they must be representable by u16b), as long as they are lesser
 * or equal to 32 if you use smooth rescaling.
 */
static void init_graphics(void)
{
	cptr tile_name;

	u16b graf_wid = 0, graf_hgt = 0;


	/* No graphics requests are made - Can't this be simpler? XXX XXX */
	if ((graf_mode_request == graf_mode) &&
	                (smooth_rescaling_request == smooth_rescaling) &&
	                !resize_request) return;

	/* Prevent further unsolicited reaction */
	resize_request = FALSE;


	/* Dispose unusable old tiles - awkward... XXX XXX */
	if ((graf_mode_request == GRAF_MODE_NONE) ||
	                (graf_mode_request != graf_mode) ||
	                (smooth_rescaling_request != smooth_rescaling)) graf_nuke();


	/* Setup parameters according to request */
	switch (graf_mode_request)
	{
		/* ASCII - no graphics whatsoever */
	default:
	case GRAF_MODE_NONE:
		{
			tile_name = NULL;
			use_graphics = arg_graphics = FALSE;

			break;
		}

		/*
		 * 8x8 tiles originally collected for the Amiga port
		 * from several contributers by Lars Haugseth, converted
		 * to 256 colours and expanded by the Z devteam
		 *
		 * Use the "old" tile assignments
		 *
		 * Dawnmist is working on it for ToME
		 */
	case GRAF_MODE_OLD:
		{
			tile_name = "8x8";
			graf_wid = graf_hgt = 8;
			ANGBAND_GRAF = "old";
			use_graphics = arg_graphics = TRUE;

			break;
		}

		/*
		 * Adam Bolt's 16x16 tiles
		 * "new" tile assignments
		 * It is updated for ToME by Andreas Koch
		 */
	case GRAF_MODE_NEW:
		{
			tile_name = "16x16";
			graf_wid = graf_hgt = 16;
			ANGBAND_GRAF = "new";
			use_graphics = arg_graphics = TRUE;

			break;
		}
	}


	/* load tiles and set them up if tiles are requested */
	if ((graf_mode_request != GRAF_MODE_NONE) &&
	                !graf_init(tile_name, graf_wid, graf_hgt))
	{
		/* Oops */
		plog("Cannot initialize graphics");

		/* reject requests */
		graf_mode_request = GRAF_MODE_NONE;
		smooth_rescaling_request = smooth_rescaling;

		/* reset graphics flags */
		use_graphics = arg_graphics = FALSE;
	}

	/* Update current graphics mode */
	graf_mode = graf_mode_request;
	smooth_rescaling = smooth_rescaling_request;

	/* Reset visuals */
	reset_visuals();
}

#endif /* USE_GRAPHICS */




/**** Term package support routines ****/


/*
 * Free data used by a term
 */
static void Term_nuke_gtk(term *t)
{
	term_data *td = t->data;


	/* Free name */
	if (td->name) string_free(td->name);

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

#ifdef USE_GRAPHICS

	/* Free tiles */
	if (td->tiles) gdk_rgb_image_destroy(td->tiles);

	/* Forget pointer */
	td->tiles = NULL;

	/* Free transparency buffer */
	if (td->trans_buf) gdk_rgb_image_destroy(td->trans_buf);

	/* Amnesia */
	td->trans_buf = NULL;

#endif /* USE_GRAPHICS */
}


/*
 * Erase the whole term.
 */
static errr Term_clear_gtk(void)
{
	term_data *td = (term_data*)(Term->data);


	/* Don't draw to hidden windows */
	if (!td->shown) return (0);

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

	/* Success */
	return (0);
}


/*
 * Erase some characters.
 */
static errr Term_wipe_gtk(int x, int y, int n)
{
	term_data *td = (term_data*)(Term->data);


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
static errr Term_text_gtk(int x, int y, int n, byte a, cptr s)
{
	term_data *td = (term_data*)(Term->data);


	/* Don't draw to hidden windows */
	if (!td->shown) return (0);

	/* Paranoia */
	g_assert(td->drawing_area->window != 0);

	/* Set foreground colour */
	term_data_set_fg(td, a);

	/* Clear the line */
	Term_wipe_gtk(x, y, n);

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

	/* Success */
	return (0);
}


/*
 * Draw software cursor at (x, y)
 */
static errr Term_curs_gtk(int x, int y)
{
	term_data *td = (term_data*)(Term->data);
	int cells = 1;


	/* Don't draw to hidden windows */
	if (!td->shown) return (0);

	/* Paranoia */
	g_assert(td->drawing_area->window != 0);

	/* Set foreground colour */
	term_data_set_fg(td, TERM_YELLOW);

#ifdef USE_DOUBLE_TILES

	/* Mogami's bigtile patch */

	/* Adjust it if wide tiles are requested */
	if (use_bigtile &&
	                (x + 1 < Term->wid) &&
	                (Term->old->a[y][x + 1] == 255))
	{
		cells = 2;
	}

#endif /* USE_DOUBLE_TILES */

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

	/* Success */
	return (0);
}


#ifdef USE_GRAPHICS

/*
 * XXX XXX Low level graphics helper
 * Draw a tile at (s_x, s_y) over one at (t_x, t_y) and store the
 * result in td->trans_buf
 *
 * XXX XXX Even if CPU's are faster than necessary these days,
 * this should be made inline. Or better, there should be an API
 * to take advantage of graphics hardware. They almost always have
 * assortment of builtin bitblt's...
 */
static void overlay_tiles_2(
        term_data *td,
        int s_x, int s_y,
        int t_x, int t_y)
{
	guint32 pix;
	int x, y;


	/* Process each row */
	for (y = 0; y < td->tile_hgt; y++)
	{
		/* Process each column */
		for (x = 0; x < td->tile_wid; x++)
		{
			/* Get an overlay pixel */
			pix = gdk_rgb_image_get_pixel(td->tiles, s_x + x, s_y + y);

			/* If it's in background color, use terrain instead */
			if (pix == td->bg_pixel)
				pix = gdk_rgb_image_get_pixel(td->tiles, t_x + x, t_y + y);

			/* Store the result in trans_buf */
			gdk_rgb_image_put_pixel(td->trans_buf, x, y, pix);
		}
	}
}


/*
 * XXX XXX Low level graphics helper
 * Draw a tile at (e_x, e_y) over one at (s_x, s_y) over another one
 * at (t_x, t_y) and store the result in td->trans_buf
 *
 * XXX XXX The same comment applies as that for the above...
 */
static void overlay_tiles_3(
        term_data *td,
        int e_x, int e_y,
        int s_x, int s_y,
        int t_x, int t_y)
{
	guint32 pix;
	int x, y;


	/* Process each row */
	for (y = 0; y < td->tile_hgt; y++)
	{
		/* Process each column */
		for (x = 0; x < td->tile_wid; x++)
		{
			/* Get an overlay pixel */
			pix = gdk_rgb_image_get_pixel(td->tiles, e_x + x, e_y + y);

			/*
			 * If it's background colour, try to use one from
			 * the second layer
			 */
			if (pix == td->bg_pixel)
				pix = gdk_rgb_image_get_pixel(td->tiles, s_x + x, s_y + y);

			/*
			 * If it's background colour again, fall back to
			 * the terrain layer
			 */
			if (pix == td->bg_pixel)
				pix = gdk_rgb_image_get_pixel(td->tiles, t_x + x, t_y + y);

			/* Store the pixel in trans_buf */
			gdk_rgb_image_put_pixel(td->trans_buf, x, y, pix);
		}
	}
}



/*
 * Low level graphics (Assumes valid input)
 *
 * Draw "n" tiles/characters starting at (x,y)
 */
static errr Term_pict_gtk(
        int x, int y, int n,
        const byte *ap, const char *cp,
        const byte *tap, const char *tcp,
        const byte *eap, const char *ecp)
{
	term_data *td = (term_data*)(Term->data);

	int i;

	int d_x, d_y;

# ifdef USE_DOUBLE_TILES

	/* Hack - remember real number of columns affected XXX XXX XXX */
	int cols;

# endif  /* USE_DOUBLE_TILES */


	/* Don't draw to hidden windows */
	if (!td->shown) return (0);

	/* Paranoia */
	g_assert(td->drawing_area->window != 0);

	/* Top left corner of the destination rect */
	d_x = x * td->font_wid;
	d_y = y * td->font_hgt;


# ifdef USE_DOUBLE_TILES

	/* Reset column counter */
	cols = 0;

# endif  /* USE_DOUBLE_TILES */

	/* Scan the input */
	for (i = 0; i < n; i++)
	{
		byte a;
		char c;
		int s_x, s_y;

		byte ta;
		char tc;
		int t_x, t_y;

		byte ea;
		char ec;
		int e_x = 0, e_y = 0;
		bool_ has_overlay;


		/* Grid attr/char */
		a = *ap++;
		c = *cp++;

		/* Terrain attr/char */
		ta = *tap++;
		tc = *tcp++;

		/* Overlay attr/char */
		ea = *eap++;
		ec = *ecp++;
		has_overlay = (ea && ec);

		/* Row and Col */
		s_y = (((byte)a & 0x7F) % tile_rows) * td->tile_hgt;
		s_x = (((byte)c & 0x7F) % tile_cols) * td->tile_wid;

		/* Terrain Row and Col */
		t_y = (((byte)ta & 0x7F) % tile_rows) * td->tile_hgt;
		t_x = (((byte)tc & 0x7F) % tile_cols) * td->tile_wid;

		/* Overlay Row and Col */
		if (has_overlay)
		{
			e_y = (((byte)ea & 0x7F) % tile_rows) * td->tile_hgt;
			e_x = (((byte)ec & 0x7F) % tile_cols) * td->tile_wid;
		}


# ifdef USE_DOUBLE_TILES

		/* Mogami's bigtile patch */

		/* Hack -- a filler for wide tile */
		if (use_bigtile && (a == 255))
		{
			/* Advance */
			d_x += td->font_wid;

			/* Ignore */
			continue;
		}

# endif  /* USE_DOUBLE_TILES */

		/* Optimise the common case: terrain == obj/mons */
		if (!use_transparency ||
		                ((s_x == t_x) && (s_y == t_y)))
		{

			/* The simplest possible case - no overlay */
			if (!has_overlay)
			{
				/* Draw the tile */
				gdk_draw_rgb_image_2(
				        TERM_DATA_DRAWABLE(td), td->gc, td->tiles,
				        s_x, s_y,
				        d_x, d_y,
				        td->tile_wid, td->tile_hgt);
			}

			/* We have to draw overlay... */
			else
			{
				/* Overlay */
				overlay_tiles_2(td, e_x, e_y, s_x, s_y);

				/* And draw the result */
				gdk_draw_rgb_image_2(
				        TERM_DATA_DRAWABLE(td), td->gc, td->trans_buf,
				        0, 0,
				        d_x, d_y,
				        td->tile_wid, td->tile_hgt);

				/* Hack -- Prevent potential display problem */
				gdk_flush();
			}

		}

		/*
		 * Since there's no masking bitblt in X,
		 * we have to do that manually...
		 */
		else
		{

			/* No overlay */
			if (!has_overlay)
			{
				/* Build terrain + masked overlay image */
				overlay_tiles_2(td, s_x, s_y, t_x, t_y);
			}

			/* With overlay */
			else
			{
				/* Ego over mon/PC over terrain */
				overlay_tiles_3(td, e_x, e_y, s_x, s_y,
				                t_x, t_y);
			}

			/* Draw it */
			gdk_draw_rgb_image_2(
			        TERM_DATA_DRAWABLE(td), td->gc, td->trans_buf,
			        0, 0,
			        d_x, d_y,
			        td->tile_wid, td->tile_hgt);

			/* Hack -- Prevent potential display problem */
			gdk_flush();
		}

		/*
		 * Advance x-coordinate - wide font fillers are taken care of
		 * before entering the tile drawing code.
		 */
		d_x += td->font_wid;

# ifdef USE_DOUBLE_TILES

		/* Add up *real* number of columns updated XXX XXX XXX */
		cols += use_bigtile ? 2 : 1;

# endif  /* USE_DOUBLE_TILES */
	}

# ifndef USE_DOUBLE_TILES

	/* Copy image from backing store if present */
	TERM_DATA_REFRESH(td, x, y, n, 1);

# else

	/* Copy image from backing store if present */
	TERM_DATA_REFRESH(td, x, y, cols, 1);

# endif  /* USE_DOUBLE_TILES */

	/* Success */
	return (0);
}

#endif /* USE_GRAPHICS */


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
static errr Term_xtra_gtk(int n, int v)
{
	/* Handle a subset of the legal requests */
	switch (n)
	{
		/* Make a noise */
	case TERM_XTRA_NOISE:
		{
			/* Beep */
			gdk_beep();

			/* Success */
			return (0);
		}

		/* Flush the output */
	case TERM_XTRA_FRESH:
		{
			/* Flush pending X requests - almost always no-op */
			gdk_flush();

			/* Success */
			return (0);
		}

		/* Process random events */
	case TERM_XTRA_BORED:
		{
			/* Process a pending event if there's one */
			CheckEvent(FALSE);

			/* Success */
			return (0);
		}

		/* Process Events */
	case TERM_XTRA_EVENT:
		{
			/* Process an event */
			CheckEvent(v);

			/* Success */
			return (0);
		}

		/* Flush the events */
	case TERM_XTRA_FLUSH:
		{
			/* Process all pending events */
			DrainEvents();

			/* Success */
			return (0);
		}

		/* Handle change in the "level" */
	case TERM_XTRA_LEVEL:
		return (0);

		/* Clear the screen */
	case TERM_XTRA_CLEAR:
		return (Term_clear_gtk());

		/* Delay for some milliseconds */
	case TERM_XTRA_DELAY:
		{
			/* sleep for v milliseconds */
			usleep(v * 1000);

			/* Done */
			return (0);
		}

		/* Get Delay of some milliseconds */
	case TERM_XTRA_GET_DELAY:
		{
			int ret;
			struct timeval tv;

			ret = gettimeofday(&tv, NULL);
			Term_xtra_long = (tv.tv_sec * 1000) + (tv.tv_usec / 1000);

			return ret;
		}

		/* Subdirectory scan */
	case TERM_XTRA_SCANSUBDIR:
		{
			DIR *directory;
			struct dirent *entry;

			scansubdir_max = 0;

			directory = opendir(scansubdir_dir);
			if (!directory) return (1);

			while ((entry = readdir(directory)) != NULL)
			{
				char file[PATH_MAX + NAME_MAX + 2];
				struct stat filedata;

				file[PATH_MAX + NAME_MAX] = 0;
				strncpy(file, scansubdir_dir, PATH_MAX);
				strncat(file, "/", 2);
				strncat(file, entry->d_name, NAME_MAX);
				if ((stat(file, &filedata) == 0) && S_ISDIR(filedata.st_mode))
				{
					string_free(scansubdir_result[scansubdir_max]);
					scansubdir_result[scansubdir_max] =
					        string_make(entry->d_name);
					++scansubdir_max;
				}
			}
		}

		/* Rename main window */
	case TERM_XTRA_RENAME_MAIN_WIN: gtk_window_set_title(GTK_WINDOW(data[0].window), angband_term_name[0]); return (0);

		/* React to changes */
	case TERM_XTRA_REACT:
		{
			/* (re-)init colours */
			init_colours();

#ifdef USE_GRAPHICS

			/* Initialise graphics */
			init_graphics();

#endif /* USE_GRAPHICS */

			/* Success */
			return (0);
		}
	}

	/* Unknown */
	return (1);
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
static void gtk_message(cptr msg)
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
static void hook_plog(cptr str)
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
static void load_font(term_data *td, cptr fontname)
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

#ifndef USE_DOUBLE_TILES

	/* Use the current font size for tiles as well */
	td->tile_wid = td->font_wid;
	td->tile_hgt = td->font_hgt;

#else /* !USE_DOUBLE_TILES */

	/* Calculate the size of tiles */
	if (use_bigtile && (td == &data[0])) td->tile_wid = td->font_wid * 2;
	else td->tile_wid = td->font_wid;
	td->tile_hgt = td->font_hgt;

#endif /* !USE_DOUBLE_TILES */
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


#ifdef USE_GRAPHICS

/*
 * Set graf_mode_request according to user selection,
 * and let Term_xtra react to the change.
 */
static void change_graf_mode_event_handler(
        gpointer user_data,
		guint user_action,
        GtkWidget *was_clicked)
{
	/* Set request according to user selection */
	graf_mode_request = (int)user_action;

	/*
	 * Hack - force redraw
	 * This induces a call to Term_xtra(TERM_XTRA_REACT, 0) as well
	 */
	Term_key_push(KTRL('R'));
}


/*
 * Set dither_mode according to user selection
 */
static void change_dith_mode_event_handler(
        gpointer user_data,
		guint user_action,
        GtkWidget *was_clicked)
{
	/* Set request according to user selection */
	dith_mode = (int)user_action;

	/*
	 * Hack - force redraw
	 */
	Term_key_push(KTRL('R'));
}


/*
 * Toggles the graphics tile scaling mode (Fast/Smooth)
 */
static void change_smooth_mode_event_handler(
        gpointer user_data,
		guint user_action,
        GtkWidget *was_clicked)
{
	/* (Try to) toggle the smooth rescaling mode */
	smooth_rescaling_request = !smooth_rescaling;

	/*
	 * Hack - force redraw
	 * This induces a call to Term_xtra(TERM_XTRA_REACT, 0) as well
	 */
	Term_key_push(KTRL('R'));
}


# ifdef USE_DOUBLE_TILES

static void change_wide_tile_mode_event_handler(
        gpointer user_data,
		guint user_action,
        GtkWidget *was_clicked)
{
	term *old = Term;
	term_data *td = &data[0];

	/* Toggle "use_bigtile" */
	use_bigtile = !use_bigtile;

	/* T.o.M.E. requires this as well */
	arg_bigtile = use_bigtile;

	/* Double the width of tiles (only for the main window) */
	if (use_bigtile)
	{
		td->tile_wid = td->font_wid * 2;
	}

	/* Use the width of current font */
	else
	{
		td->tile_wid = td->font_wid;
	}

	/* Need to resize the tiles */
	resize_request = TRUE;

	/* Activate the main window */
	Term_activate(&td->t);

	/* Resize the term */
	Term_resize(td->cols, td->rows);

	/* Activate the old term */
	Term_activate(old);

	/* Hack - force redraw XXX ??? XXX */
	Term_key_push(KTRL('R'));
}

# endif  /* USE_DOUBLE_TILES */


/*
 * Toggles the boolean value of use_transparency
 */
static void change_trans_mode_event_handler(
        gpointer user_data,
		guint user_aciton,
        GtkWidget *was_clicked)
{
	/* Toggle the transparency mode */
	use_transparency = !use_transparency;

	/* Hack - force redraw */
	Term_key_push(KTRL('R'));
}

#endif /* USE_GRAPHICS */


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
	switch ((uint) event->keyval)
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
		Term_activate(&td->t);

		/* Resize if necessary */
		if ((td->cols != old_cols) || (td->rows != old_rows))
			(void)Term_resize(td->cols, td->rows);

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
		Term_activate(&td->t);

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
	term *t = &td->t;
	char *p;

	td->cols = 80;
	td->rows = 24;

	/* Initialize the term */
	term_init(t, td->cols, td->rows, 1024);

	/* Store the name of the term */
	td->name = string_make(angband_term_name[i]);

	/* Instance names should start with a lowercase letter XXX */
	for (p = (char *)td->name; *p; p++) *p = tolower(*p);

	/* Use a "soft" cursor */
	t->soft_cursor = TRUE;

	/* Erase with "white space" */
	t->attr_blank = TERM_WHITE;
	t->char_blank = ' ';

	t->xtra_hook = Term_xtra_gtk;
	t->text_hook = Term_text_gtk;
	t->wipe_hook = Term_wipe_gtk;
	t->curs_hook = Term_curs_gtk;
#ifdef USE_GRAPHICS
	t->pict_hook = Term_pict_gtk;
#endif /* USE_GRAPHICS */
	t->nuke_hook = Term_nuke_gtk;

	/* Save the data */
	t->data = td;

	/* Activate (important) */
	Term_activate(t);

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

#ifdef USE_GRAPHICS

	/* "Graphics" submenu */
	{ "/Options/Graphics", NULL,
	  NULL, 0, "<Branch>", NULL },
	{ "/Options/Graphics/None", NULL,
	  change_graf_mode_event_handler, GRAF_MODE_NONE, "<CheckItem>", NULL },
	{ "/Options/Graphics/Old", NULL,
	  change_graf_mode_event_handler, GRAF_MODE_OLD, "<CheckItem>", NULL },
	{ "/Options/Graphics/New", NULL,
	  change_graf_mode_event_handler, GRAF_MODE_NEW, "<CheckItem>", NULL },
# ifdef USE_DOUBLE_TILES
	{ "/Options/Graphics/sep3", NULL,
	  NULL, 0, "<Separator>", NULL },
	{ "/Options/Graphics/Wide tiles", NULL,
	  change_wide_tile_mode_event_handler, 0, "<CheckItem>", NULL },
# endif  /* USE_DOUBLE_TILES */
	{ "/Options/Graphics/sep1", NULL,
	  NULL, 0, "<Separator>", NULL },
	{ "/Options/Graphics/Dither if <= 8bpp", NULL,
	  change_dith_mode_event_handler, GDK_RGB_DITHER_NORMAL, "<CheckItem>", NULL },
	{ "/Options/Graphics/Dither if <= 16bpp", NULL,
	  change_dith_mode_event_handler, GDK_RGB_DITHER_MAX, "<CheckItem>", NULL },
	{ "/Options/Graphics/sep2", NULL,
	  NULL, 0, "<Separator>", NULL },
	{ "/Options/Graphics/Smoothing", NULL,
	  change_smooth_mode_event_handler, 0, "<CheckItem>", NULL },
	{ "/Options/Graphics/Transparency", NULL,
	  change_trans_mode_event_handler, 0, "<CheckItem>", NULL },

#endif /* USE_GRAPHICS */

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
		term_entry[i].path = (gchar*)string_make(buf);

		/* XXX XXX Build the real path name to the entry */
		strnfmt(buf, 64, "/Options/Font/%s", angband_term_name[i]);

		/* XXX XXX Store it in the menu definition */
		font_entry[i].path = (gchar*)string_make(buf);
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
		if (term_entry[i].path) string_free((cptr)term_entry[i].path);

		/* XXX XXX Free Font menu path */
		if (font_entry[i].path) string_free((cptr)font_entry[i].path);
	}
}


/*
 * Find widget corresponding to path name
 * return NULL on error
 */
static GtkWidget *get_widget_from_path(cptr path)
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
void enable_menu_item(cptr path, bool_ enabled)
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
void check_menu_item(cptr path, bool_ checked)
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


#ifdef USE_GRAPHICS

/*
 * Update the "Graphics" submenu
 */
static void graf_menu_update_handler(
        GtkWidget *widget,
        gpointer user_data)
{
	/* Update menu items */
	check_menu_item(
	        "<Angband>/Options/Graphics/None",
	        (graf_mode == GRAF_MODE_NONE));
	check_menu_item(
	        "<Angband>/Options/Graphics/Old",
	        (graf_mode == GRAF_MODE_OLD));
	check_menu_item(
	        "<Angband>/Options/Graphics/New",
	        (graf_mode == GRAF_MODE_NEW));

#ifdef USE_DOUBLE_TILES

	check_menu_item(
	        "<Angband>/Options/Graphics/Wide tiles",
	        use_bigtile);

#endif /* USE_DOUBLE_TILES */

	check_menu_item(
	        "<Angband>/Options/Graphics/Dither if <= 8bpp",
	        (dith_mode == GDK_RGB_DITHER_NORMAL));
	check_menu_item(
	        "<Angband>/Options/Graphics/Dither if <= 16bpp",
	        (dith_mode == GDK_RGB_DITHER_MAX));

	check_menu_item(
	        "<Angband>/Options/Graphics/Smoothing",
	        smooth_rescaling);

	check_menu_item(
	        "<Angband>/Options/Graphics/Transparency",
	        use_transparency);
}

#endif /* USE_GRAPHICS */


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

#ifdef USE_GRAPHICS

	/* Access Graphics menu */
	widget = get_widget_from_path("<Angband>/Options/Graphics");

	/* Paranoia */
	g_assert(widget != NULL);
	g_assert(GTK_IS_MENU(widget));

	/* Assign callback */
	gtk_signal_connect(
	        GTK_OBJECT(widget),
	        "show",
	        GTK_SIGNAL_FUNC(graf_menu_update_handler),
	        NULL);

#endif /* USE_GRAPHICS */
}


/*
 * Create Gtk widgets for a terminal window and set up callbacks
 */
static void init_gtk_window(term_data *td, int i)
{
	GtkWidget *menu_bar = NULL, *box;
	cptr font;

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
static void hook_quit(cptr str)
{
	/* Free menu paths dynamically allocated */
	free_menu_paths();

# ifdef USE_GRAPHICS

	/* Free pathname string */
	if (ANGBAND_DIR_XTRA_GRAF) string_free(ANGBAND_DIR_XTRA_GRAF);

# endif  /* USE_GRAPHICS */

	/* Terminate the program */
	gtk_exit(0);
}


/*
 * Initialization function
 */
errr init_gtk2(int argc, char **argv)
{
	int i;


	/* Initialize the environment */
	gtk_init(&argc, &argv);

	/* Activate hooks - Use gtk/glib interface throughout */
	ralloc_aux = hook_ralloc;
	rnfree_aux = hook_rnfree;
	quit_aux = hook_quit;
	core_aux = hook_quit;

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

#ifdef USE_GRAPHICS

		/* Requests "old" graphics */
		if (streq(argv[i], "-o"))
		{
			graf_mode_request = GRAF_MODE_OLD;
			continue;
		}

		/* Requests "new" graphics */
		if (streq(argv[i], "-g"))
		{
			graf_mode_request = GRAF_MODE_NEW;
			continue;
		}

# ifdef USE_DOUBLE_TILES

		/* Requests wide tile mode */
		if (streq(argv[i], "-w"))
		{
			use_bigtile = TRUE;
			arg_bigtile = TRUE;
			continue;
		}

# endif  /* USE_DOUBLE_TILES */


		/* Enable transparency effect */
		if (streq(argv[i], "-t"))
		{
			use_transparency = TRUE;
			continue;
		}

		/* Disable smooth rescaling of tiles */
		if (streq(argv[i], "-s"))
		{
			smooth_rescaling_request = FALSE;
			continue;
		}

#endif /* USE_GRAPHICS */

		/* None of the above */
		plog_fmt("Ignoring option: %s", argv[i]);
	}

#ifdef USE_GRAPHICS

	{
		char path[1024];

		/* Build the "graf" path */
		path_build(path, 1024, ANGBAND_DIR_XTRA, "graf");

		/* Allocate the path */
		ANGBAND_DIR_XTRA_GRAF = string_make(path);
	}

#endif /* USE_GRAPHICS */

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
	Term_activate(&data[0].t);

	/* Activate more hook */
	plog_aux = hook_plog;

	/* It's too early to set this, but cannot do so elsewhere XXX XXX */
	game_in_progress = TRUE;

	/* Success */
	return (0);
}

#endif /* USE_GTK2 */

/*
 *
 * This file contains routines for maiming bitmaps as well as other
 * supplemental routines, all for SDL.
 *
 * Copyright 2001 Gregory Velichansky (hmaon@bumba.net)
 * You may use it under the terms of the standard Angband license (below) or
 * the GNU General Public License (GPL) Version 2 or greater. (see below)
 *
 * The Angband license states:
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 * 
 * The GNU GPL notice:
   main-sdl.c - SDL (http://libsdl.org) Term implementation for Angband.
	Copyright (C) 2001  Gregory Velichansky (hmaon@bumba.net)
 
	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.
 
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
 
	You should have received a copy of the GNU General Public License
	along with this program; if not, please see 
	http://www.gnu.org/copyleft/gpl.html
 
 
	Please see the file COPYING for more detail regarding Angband's current
	license situation.
*/


#include "angband.h"

#if defined(USE_ISO) || defined(USE_LUA_GUI)


#include "SDL/SDL.h"
#include <string.h>
#include <math.h> /* for scaling blits */
//#include "bits/nan.h"

/*
 *
 * Supplemental SDL bitmap manipulation functions.
 *
 * These could be moved to a separate file. In mai?-x11.c, similar routines
 * are separate from the main display module implementation.
 *
 */


/* The most pedantic-a%& getpixel and putpixel ever, hopefully. */
/* There may still be endianness bugs! These will be fixed after adequte testing. XXX XXX XXX */
inline errr SDL_GetPixel (SDL_Surface *f, Uint32 x, Uint32 y, Uint8 *r, Uint8 *g, Uint8 *b)
{
	/*const Uint32 mask[] = {0x0, 0xff, 0xffff, 0xffffff, 0xffffffff};*/
	Uint32 pixel;

	Uint8 *pp;

	int n;  /* general purpose 'n'. */

	if (f == NULL) return -1;

	pp = (Uint8 *) f->pixels;

	if (x >= f->w || y >= f->h) return -1;

	pp += (f->pitch * y);

	pp += (x * f->format->BytesPerPixel);

	/* we do not lock the surface here, it would be inefficient XXX */
	/* this reads the pixel as though it was a big-endian integer XXX */
	/* I'm trying to avoid reading part the end of the pixel data by
	 * using a data-type that's larger than the pixels */
	for (n = 0, pixel = 0; n < f->format->BytesPerPixel; ++n, ++pp)
	{
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
		pixel >>= 8;
		pixel |= *pp << (f->format->BitsPerPixel - 8);
#else
pixel |= *pp;
		pixel <<= 8;
#endif
	}

	SDL_GetRGB(pixel, f->format, r, g, b);
	return 0;
}

/* This function looks remarkably similar to the one above. Yes, it's cut
 * and paste. */
inline errr SDL_PutPixel (SDL_Surface *f, Uint32 x, Uint32 y, Uint8 r, Uint8 g, Uint8 b)
{
	Uint32 pixel;

	Uint8 *pp;

	int n;

	if (f == NULL) return -1;

	pp = (Uint8 *) f->pixels;

	if (x >= f->w || y >= f->h) return -1;

	pp += (f->pitch * y);

	pp += (x * f->format->BytesPerPixel);

	pixel = SDL_MapRGB(f->format, r, g, b);

	for (n = 0; n < f->format->BytesPerPixel; ++n, ++pp)
	{
		*pp = (Uint8) (pixel & 0xFF);
		pixel >>= 8;
	}

	return 0;
}


/* This routine performs a scaling blit. It will shrink and magnify. :) */
/* It uses floating point arithmetic (because I am lazy) so it's not too fast
 * but I only intend for it to be used in pre-processing, that is image 
 * processing at load time. It's fast enough for that, at least.
 * Actually on my machine it IS fast enough to scale fonts and bitmaps
 * in real-time. :) 
 * This routine uses a weighted average, the weight being based on overlapping
 * pixel area.
 */
inline errr SDL_ScaleBlit(SDL_Surface *src, SDL_Rect *sr, SDL_Surface *dst, SDL_Rect *dr)
{
	Uint8 r, g, b;

	float rs, gs, bs;  /* sums */

	float area;

	float sx, sy;  /* current source x and y */
	float dsx, dsy;  /* source increment, per increment of 1 in destination */

	float wsx, wsy;
	/* width of source box. Equal to dsx,dsy except when either of then are
	 * smaller than 1. This is a hack for smoother magnification. XXX */


	float x, y;  /* x and y in sub-area */

	Uint32 tx, ty;  /* "to" x and y */
	Uint32 lx, ly;

	float w, e, n, s;  /* some temporary variables, named after orientations */


	if (src == NULL || sr == NULL || dst == NULL || dr == NULL) return -1;

	/* these are meaningless and would cause a divide by zero: */
	if (!dr->w || !dr->h) return -1;

	wsx = dsx = ((float) sr->w) / dr->w;
	if (wsx < 1) wsx = 1;
	wsy = dsy = ((float) sr->h) / dr->h;
	if (wsy < 1) wsy = 1;

	lx = dr->x + dr->w;
	ly = dr->y + dr->h;

	area = wsx * wsy;



	for (ty = dr->y, sy = (float)sr->y; ty < ly; ++ty, sy += dsy)
	{
		for (tx = dr->x, sx = (float)sr->x; tx < lx; ++tx, sx += dsx)
		{
			rs = gs = bs = 0.0;
			for (y = floor(sy) - 1; ceil(sy + wsy) > y; ++y)
			{
				for (x = floor(sx) - 1; ceil(sx + wsx) > x; ++x)
				{
					w = (x > sx) ? 0 : sx - x;
					n = (y > sy) ? 0 : sy - y;

					e = (sx + wsx >= x + 1) ? 1 : sx + wsx - x;
					s = (sy + wsy >= y + 1) ? 1 : sy + wsy - y;

					if (w > e || s < n ) continue;

#define gsx ((Uint32)x >= sr->x+sr->w ? sr->x+sr->w-1 : (Uint32)x)
#define gsy ((Uint32)y >= sr->y+sr->h ? sr->y+sr->h-1 : (Uint32)y)
					SDL_GetPixel (src, gsx, gsy, &r, &g, &b);



					rs += (e - w) * (s - n) * (float)r;
					gs += (e - w) * (s - n) * (float)g;
					bs += (e - w) * (s - n) * (float)b;
				}
			}
			rs /= area;
			gs /= area;
			bs /= area;

			if (rs >= 256.0 || gs >= 256.0 || bs > 256.0)
			{
				plog("weighted average error!");
				plog(format("Values: %f, %f, %f\n", rs, gs, bs));
				/**((char *)0) = 0;*/
			}
			if (rs > 255.0) rs = 255.0;
			if (gs > 255.0) gs = 255.0;
			if (bs > 255.0) bs = 255.0;

			r = (Uint8)rs;
			g = (Uint8)gs;
			b = (Uint8)bs;

			SDL_PutPixel (dst, tx, ty, r, g, b);
		}
	}

	return 0;
#undef gsx
#undef gsy
}


/* Integer math version of SDL_ScaleBlit().
 * Where necessary, a number uses the 16 high bits for the integer
 * and the 16 low bits for the decimal portion.
 *
 * eg:
 * float a = (float) (b >> 16) + (b & 0xFFFF)/65536.0;
 */

inline Uint32 ifloor(Uint32 i)
{
	return i & 0xFFFF0000;
}

inline Uint32 iceil(Uint32 i)
{
	return (i & 0xFFFF) ? i : ifloor(i) + (1 << 16);
}


errr SDL_FastScaleBlit(SDL_Surface *src, SDL_Rect *sr, SDL_Surface *dst, SDL_Rect *dr)
{
	Uint8 r, g, b;
	Uint32 rs, gs, bs;  /* sums. */

	/* temp storage for large int multiplies. Uint64 doen't exist anywhere */
	double farea;
	Uint32 area;

	Uint32 sx, sy;
	Uint32 dsx, dsy;

	Uint32 wsx, wsy;

	Uint32 x, y;  /* x and y, for sub-area */

	Uint32 tx, ty;  /* normal integers */
	Uint32 lx, ly;  /* normal integers */

	Uint32 w, e, n, s;  /* temp variables, named after compass directions */

	if (src == NULL || sr == NULL || dst == NULL || dr == NULL) return -1;

	if (!dr->w || !dr->h) return -1;


	/* TODO FIXME check for possible overflows! */

	wsx = dsx = (sr->w << 16) / dr->w;
	if (!(wsx & 0xFFFF0000)) wsx = 1 << 16;
	wsy = dsy = (sr->h << 16) / dr->h;
	if (!(wsy & 0xFFFF0000)) wsy = 1 << 16;

	lx = dr->x + dr->w;
	ly = dr->y + dr->h;

	/* lazy multiplication. Hey, it's only once per blit. :P */
	farea = ((double)wsx) * ((double)wsy);
	farea /= (double)(1 << 16);
	area = (Uint32) farea;

	/* For optimization, those setup routines should be moved into
	 * SDL_ScaleTiledBitmap() for that function.
	 */

	for (ty = dr->y, sy = sr->y << 16; ty < ly; ++ty, sy += dsy)
	{
		for (tx = dr->x, sx = sr->x << 16; tx < lx; ++tx, sx += dsx)
		{
			rs = gs = bs = 0;
			for (y = ifloor(sy); iceil(sy + wsy) > y; y += (1 << 16))
			{
				for (x = ifloor(sx); iceil(sx + wsx) > x; x += (1 << 16))
				{
					w = (x > sx) ? 0 : sx - x;
					n = (y > sy) ? 0 : sy - y;

					e = (sx + wsx >= x + (1 << 16)) ? 1 << 16 : sx + wsx - x;
					s = (sy + wsy >= y + (1 << 16)) ? 1 << 16 : sy + wsy - y;

					if (w > e || s < n) continue;

#define gsx ((x >> 16) >= sr->x+sr->w ? sr->x+sr->w-1 : x >> 16)
#define gsy ((y >> 16) >= sr->y+sr->h ? sr->y+sr->h-1 : y >> 16)

					SDL_GetPixel (src, gsx, gsy, &r, &g, &b);

					rs += ((e - w) >> 8) * ((s - n) >> 8) * r;
					gs += ((e - w) >> 8) * ((s - n) >> 8) * g;
					bs += ((e - w) >> 8) * ((s - n) >> 8) * b;
				}
			}
			rs /= area;
			gs /= area;
			bs /= area;

			if (rs >= 256 || gs >= 256 || bs >= 256)
			{
				plog("fixed point weighted average overflow!");
				plog(format("Values: %d, %d, %d\n", rs, gs, bs));
			}

			r = (Uint8) rs;
			g = (Uint8) gs;
			b = (Uint8) bs;

			SDL_PutPixel (dst, tx, ty, r, g, b);
		}
	}

	return 0;
#undef gsx
#undef gsy
}



#if 0 /* the procedure above is a more generalized version of the one below */

/* The following is a function to perform a Blit while magnifying */
/* Anti-aliasing is performed. :) */
/* It is probably very SLOW on most systems. Use it for pre-processing. XXX */
/* a Blit while shrinking is handled by a different function */
errr SDL_StretchBlit(SDL_Surface *src, SDL_Rect *sr, SDL_Surface *dst, SDL_Rect *dr)
{
	double sx, sy;  /* current source x and y */
	Uint32 isx, isy;  /* temp. values for convenience in calculation code */
	double dsx, dsy;  /* source increment, per increment of 1 in destination */
	double wx, wy;  /* temp. weight values for the color mixing calculations */
	double weight;  /* temp. weight of pixel */

	/* coordinates to get pixels from: ... */
#undef gsx
#define gsx (isx >= sr->x+sr->w ? sr->x+sr->w-1 : isx)
#undef gsy
#define gsy (isy >= sr->y+sr->h ? sr->y+sr->h-1 : isy)


	Uint32 tx, ty;  /* "to" x and y. "dx, dy" would be too confusing. */
	Uint32 lx, ly;  /* end x and y in destination, not inclusive */

	double r, g, b;  /* temporary values on which we perform calculations */
	/*double s;*/ /* scale factor calculation thing. gross hack. don't ask. */
	Uint8 ir, ig, ib;  /* same here. */

	if (src == NULL || sr == NULL || dst == NULL || dr == NULL) return -1;

	/* these are meaningless and would cause a divide by zero: */
	if (!dr->w || !dr->h) return -1;

	dsx = ((double) sr->w) / dr->w;
	dsy = ((double) sr->h) / dr->h;

	lx = dr->x + dr->w;
	ly = dr->y + dr->h;

	for (ty = dr->y, sy = (double)sr->y; ty < ly; ++ty, sy += dsy)
	{
		for (tx = dr->x, sx = (double)sr->x; tx < lx; ++tx, sx += dsx)
		{
			/* here we must consider four pixels and mix them together */
			/* the further away we are from a pixel, the less weight it has
			 * when we mix in its color. Hence the "1 - hypot(..." etc.
			 * Actually, no. Let's not use hypot().
			 */
			/*
			 * upper left pixel 
			 */
			wx = ((floor(sx) + 1) - sx);
			wy = ((floor(sy) + 1) - sy);

			isx = (Uint32) floor(sx);
			isy = (Uint32) floor(sy);

			if (SDL_GetPixel(src, gsx, gsy, &ir, &ig, &ib)) return -1;

			weight = wx * wy;
			/* the area of the overlap of our hypothetical and real pixel!!! */
			if (weight < 1 / 1024.0) weight = 0;
			r = weight * (double)ir;
			g = weight * (double)ig;
			b = weight * (double)ib;
			/*s = weight * 255.0;*/

			/*
			 * upper right pixel 
			 */
			wx = 1 - wx;
			isx += 1;

			if (SDL_GetPixel(src, gsx, gsy, &ir, &ig, &ib)) return -1;

			weight = wx * wy;
			if (weight < 1 / 1024.0) weight = 0;
			r += weight * (double)ir;
			g += weight * (double)ig;
			b += weight * (double)ib;
			/*s += weight * 255.0;*/

			/*
			 * lower right pixel 
			 */
			wy = 1 - wy;
			isy += 1;

			if (SDL_GetPixel(src, gsx, gsy, &ir, &ig, &ib)) return -1;

			weight = wx * wy;
			if (weight < 1 / 1024.0) weight = 0;
			r += weight * (double)ir;
			g += weight * (double)ig;
			b += weight * (double)ib;
			/*s += weight * 255.0;*/

			/*
			 * lower left pixel
			 */
			wx = 1 - wx;
			isx -= 1;

			if (SDL_GetPixel(src, gsx, gsy, &ir, &ig, &ib)) return -1;

			weight = wx * wy;
			if (weight < 1 / 1024.0) weight = 0;
			r += weight * (double)ir;
			g += weight * (double)ig;
			b += weight * (double)ib;
			/*s += weight * 255.0;*/


			/*
			r = 255 * (r/s);
			g = 255 * (g/s);
			b = 255 * (b/s);
			*/

			if (r >= 256.0 || g >= 256.0 || b > 256.0)
			{
				plog("mixing error!");
				plog(format("Values: %f, %f, %f\n", (double)r, (double)g, (double)b));
				/**((char *)0) = 0;*/
			}
			if (r > 255.0) r = 255.0;
			if (g > 255.0) g = 255.0;
			if (b > 255.0) b = 255.0;
			ir = (Uint8) r;
			ig = (Uint8) g;
			ib = (Uint8) b;

			SDL_PutPixel(dst, tx, ty, ir, ig, ib);
		}
	}

	return 0;
}

#endif




/* This function will take an SDL_Surface, allocate a new surface to hold
 * the resized surface, perform the scaling operation, free the old surface
 * and return the new one. This behaviour is vaguely modeled after C library
 * string functions. Returns NULL on grievous errors! 
 *
 * The scaling operation is performed one or more times to accomodate
 * images comprised by a number of sub-images whose edges must not be blurred
 * with the edges of adjacent sub-images. (Think fonts and tile sets.)
 *
 * If t_oldw and t_oldh are set to src->w and src->h respectively
 *
 * t_oldw, t_oldh are the size of the old tiles
 */
SDL_Surface *SDL_ScaleTiledBitmap (SDL_Surface *src,
                                   Uint32 t_oldw,
                                   Uint32 t_oldh,
                                   Uint32 t_neww,
                                   Uint32 t_newh,
                                   int dealloc_src)

{
	SDL_Surface *dst;
	SDL_Rect sr, dr;
	Uint32 x, y;
	Uint32 nx, ny;

	if (!t_oldw || !t_oldh || !t_neww || !t_newh || !src) return NULL;  /*dummy!*/

	if (t_oldw == t_neww && t_oldh == t_newh) return src;  /* OK... */

	/* Get the number of tiles in the image.
	 * Any possible clipped tiles at the edges are ignored.
	 */
	nx = src->w / t_oldw;
	ny = src->h / t_oldh;

	/* Allocate a new SDL_Surface of appropriate size, with settings otherwise
	 * identical to src.
	 */
	dst = SDL_CreateRGBSurface(src->flags,
	                           nx * t_neww,
	                           ny * t_newh,
	                           /*src->format->BitsPerPixel,*/
	                           16,
	                           src->format->Rmask,
	                           src->format->Gmask,
	                           src->format->Bmask,
	                           src->format->Amask);


	for (y = 0; y < ny; ++y)
	{
		for (x = 0; x < nx; ++x)
		{
			sr.w = t_oldw;
			sr.h = t_oldh;
			sr.x = x * t_oldw;
			sr.y = y * t_oldh;

			dr.w = t_neww;
			dr.h = t_newh;
			dr.x = x * t_neww;
			dr.y = y * t_newh;

			/*printf("%d,%d -> %d,%d   (%d,%d -> %d, %d)\n", sr.x, sr.y, dr.x, dr.y, sr.w, sr.h, dr.w, dr.h);*/

			/* scale-blit one tile and check for error
			 * although SDl_ScaleBlit() might not have any errors to return.
			 */
			if (SDL_FastScaleBlit(src, &sr, dst, &dr)) return NULL;
		}
	}

	if (dealloc_src) SDL_FreeSurface(src);

	return dst;
}


/* The following function will extract height and width info from a filename
 * such as 16x16.xyz or 8X13.bar or even argle8ook16.foo
 *
 * I realize now that it's also useful for reading integers out of an argument
 * such as --fooscale1=2
 */

errr strtoii(const char *str, Uint32 *w, Uint32 *h)
{
	char buf[1024];
	char *s = buf;
	char *tok;
	char *numeric = "0123456789";

	size_t l;  /* length of numeric string */

	if (!str || !w || !h) return -1;

	if (strlen(str) < 3) return -1;  /* must have room for at least "1x1" */

	strncpy(buf, str, 1023);
	buf[1023] = '\0';

	tok = strpbrk(buf, numeric);
	if (!tok) return -1;

	l = strspn(tok, numeric);
	if (!l) return -1;

	tok[l] = '\0';

	s = tok + l + 1;

	if (!sscanf(tok, "%d", w)) return -1;

	/* next token */
	tok = strpbrk(s, numeric);
	if (!tok) return -1;

	l = strspn(tok, numeric);
	if (!l) return -1;

	tok[l] = '\0';
	/* no need to set s since this is the last token */

	if (!sscanf(tok, "%d", h)) return -1;

	return 0;

}




char *formatsdlflags(Uint32 flags)
{
	return format ("%s%s%s%s%s%s%s%s%s%s (%x)",
	               (flags & SDL_HWSURFACE) ? "SDL_HWSURFACE " : "",
	               (flags & SDL_ANYFORMAT) ? "SDL_ANYFORMAT " : "",
	               (flags & SDL_HWPALETTE) ? "SDL_HWPALETTE " : "",
	               (flags & SDL_DOUBLEBUF) ? "SDL_DOUBLEBUF " : "",
	               (flags & SDL_FULLSCREEN) ? "SDL_FULLSCREEN " : "",
	               (flags & SDL_RESIZABLE) ? "SDL_RESIZABLE " : "",
	               (flags & SDL_HWACCEL) ? "SDL_HWACCEL " : "",
	               (flags & SDL_SRCCOLORKEY) ? "SDL_SRCCOLRKEY " : "",
	               (flags & SDL_RLEACCEL) ? "SDL_RLEACCEL " : "",
	               (flags & SDL_SRCALPHA) ? "SDL_SRCALPHA " : "",
	               flags);
};






/* A lot of code for handling keystrokes follow. */


typedef struct sdl_keymapt sdl_keymapt;

struct sdl_keymapt
{
	SDLKey k;  /* what we get from SDL */
	char *s;  /* what we feed to the Term_keypress */
	char *ctrl;  /* what if CTRL is pressed? (NULL if the same) */
	char *shift;  /* what if SHIFT is pressed? */
};

/* XXX XXX XXX the following keymap sucks. More comments below. */
sdl_keymapt sdl_keymap[] =
{
	/*{SDLK_UP, "[A", "Oa", "Ox"},
	{SDLK_DOWN, "[B", "Ob", "Or"},
	{SDLK_RIGHT, "[C", "Oc", "Ot"},
	{SDLK_LEFT, "[D", "Od", "Ov"},
	{SDLK_INSERT, "[2~", "[2^", "Op"},
	{SDLK_HOME, "[1~", "[1^", "Ow"},
	{SDLK_END, "[4~", "[4^", "Oq"},
	{SDLK_PAGEUP, "[5~", "[5^", "Oy"},
	{SDLK_PAGEDOWN, "[6~", "[6^", "Os"},*/
	{SDLK_F1, "[[A", NULL, NULL},
	{SDLK_F2, "[[B", NULL, NULL},
	{SDLK_F3, "[[C", NULL, NULL},
	{SDLK_F4, "[[D", NULL, NULL},
	{SDLK_F5, "[[E", NULL, NULL},
	{SDLK_F6, "[[17~", NULL, NULL},
	{SDLK_F7, "[[18~", NULL, NULL},
	{SDLK_F8, "[[19~", NULL, NULL},
	{SDLK_F9, "[[20~", NULL, NULL},
	{SDLK_F10, "[[21~", NULL, NULL},
	{SDLK_F11, "[[23~", NULL, NULL},
	{SDLK_F12, "[[24~", NULL, NULL},
	/* I have no machines with F13, F14, F15. Is that a Sun thing? */
	{SDLK_F13, "", NULL, NULL},
	{SDLK_F14, "", NULL, NULL},
	{SDLK_F15, "", NULL, NULL},
	{SDLK_RSHIFT, "", NULL, NULL},
	{SDLK_LSHIFT, "", NULL, NULL},
	{SDLK_RALT, "", NULL, NULL},
	{SDLK_LALT, "", NULL, NULL},
	{SDLK_RCTRL, "", NULL, NULL},
	{SDLK_LCTRL, "", NULL, NULL},
	{SDLK_RMETA, "", NULL, NULL},
	{SDLK_LMETA, "", NULL, NULL},
	{SDLK_NUMLOCK, "", NULL, NULL},
	{SDLK_CAPSLOCK, "", NULL, NULL},
	{SDLK_SCROLLOCK, "", NULL, NULL},
	{SDLK_LSUPER, "", NULL, NULL},
	{SDLK_RSUPER, "", NULL, NULL},
	{SDLK_HELP, "?", NULL, NULL},
	{SDLK_PRINT, "", NULL, NULL},
	{SDLK_SYSREQ, "", NULL, NULL},
	{SDLK_BREAK, "", NULL, NULL},
	{SDLK_MENU, "", NULL, NULL},
	{SDLK_POWER, "", NULL, NULL},
	{SDLK_EURO, "", NULL, NULL},
	{SDLK_0, "0", NULL, ")"},  /* XXX XXX XXX The CTRL-number keys need to be */
	{SDLK_1, "1", NULL, "!"},  /* defined since they represent digging for    */
	{SDLK_2, "2", NULL, "@"},  /* some people!. Really, this whole table      */
	{SDLK_3, "3", NULL, "#"},  /* should be replaced with something cleaner   */
	{SDLK_4, "4", NULL, "$"},  /* and an SDL pref file should be created.     */
	{SDLK_5, "5", NULL, "%"},
	{SDLK_6, "6", NULL, "^"},
	{SDLK_7, "7", NULL, "&"},
	{SDLK_8, "8", NULL, "*"},
	{SDLK_9, "9", NULL, "("},
	{SDLK_SEMICOLON, ";", NULL, ":"},
	{SDLK_COMMA, ",", NULL, "<"},
	{SDLK_PERIOD, ".", NULL, ">"},
	{SDLK_BACKSLASH, "\\", NULL, "|"},
	{SDLK_BACKQUOTE, "`", NULL, "~"},
	{SDLK_LEFTBRACKET, "[", NULL, "{"},
	{SDLK_RIGHTBRACKET, "]", NULL, "}"},
	{SDLK_MINUS, "-", NULL, "_"},
	{SDLK_EQUALS, "=", NULL, "+"},
	{SDLK_SLASH, "/", NULL, "?"},
	{SDLK_UNKNOWN, NULL, NULL, NULL} /* terminator */
};

void Multikeypress(char *k)
{
	printf("zog : '%s'\n", k);
	while (*k) Term_keypress(*k++);
}

int IsMovement(SDLKey k)
{
	switch (k)
	{
	case SDLK_UP:
	case SDLK_DOWN:
	case SDLK_RIGHT:
	case SDLK_LEFT:
	case SDLK_INSERT:
	case SDLK_HOME:
	case SDLK_END:
	case SDLK_PAGEUP:
	case SDLK_PAGEDOWN:
	case SDLK_KP0:
	case SDLK_KP1:
	case SDLK_KP2:
	case SDLK_KP3:
	case SDLK_KP4:
	case SDLK_KP5:
	case SDLK_KP6:
	case SDLK_KP7:
	case SDLK_KP8:
	case SDLK_KP9:
		return TRUE;

	default:
		return FALSE;
	}
	return 1234567;  /* all good children go to heaven */
}


char *SDL_keysymtostr(SDL_keysym *ks)
{
#ifdef bufsize
#error bufsize steps on previous define!
#endif
#define bufsize 1024
	int bufused = 0;

	/* I am returning a pointer to the below variable.
	 * I /think/ this is legal but I am not sure!  XXX XXX XXX
	 * It certainly seems to work fine, at least under GCC.
	 * It can easily be changed to a pointer passed as an argument.
	 */
	static char buf[bufsize];
	Uint8 ch;

	/* cat for strings and app[end] for characters */
#define sdlkcat(a) strncat(buf,(a),bufsize-bufused-1); bufused+=strlen((a));
#define sdlkapp(a) if(bufused<bufsize-1) { buf[bufused]=a; buf[bufused+1]='\0'; bufused++; }

	buf[0] = '\0';

	//	printf("%c\n", ks->unicode);


	if (IsMovement(ks->sym))
	{
		sprintf(buf, "%c%s%s%s%s_%lX%c", 31,
		        ks->mod & KMOD_CTRL ? "N" : "",
		        ks->mod & KMOD_SHIFT ? "S" : "",
		        "",  /* for future expansion. */
		        ks->mod & KMOD_ALT ? "M" : "",
		        (unsigned long) ks->sym, 13);
		ch = 0;
	}
	else
	{
		if (ks->mod & KMOD_ALT)
		{
			sdlkapp('');
		}
		ch = ks->unicode;

	}

	if (ch) sdlkapp(ch);

	return buf;


#undef bufsize
#undef sdlkcat
#undef sdlkapp

} /* SDL_keystring */



/* Cursor hack, for testing of ideas. XXX XXX XXX */

SDL_Surface *sdl_screen_cursor = NULL;
SDL_Rect sdl_screen_cursor_sr;


/* it must be true that w * h * 4 <= maxUint32 */

errr SDL_init_screen_cursor(Uint32 w, Uint32 h)
{
	Uint32 i;

	sdl_screen_cursor_sr.x = sdl_screen_cursor_sr.y = 0;
	sdl_screen_cursor_sr.w = w;
	sdl_screen_cursor_sr.h = h;

	sdl_screen_cursor = NULL;
	sdl_screen_cursor = SDL_CreateRGBSurface(SDL_SRCALPHA, w, h, 32,
	                    0xff000000,
	                    0x00ff0000,
	                    0x0000ff00,
	                    0x00000000);

	if (!sdl_screen_cursor) return -1;

	SDL_SetAlpha(sdl_screen_cursor, SDL_SRCALPHA | SDL_RLEACCEL, 0x80);
	for (i = 0; i < w*h*4; ++i)
	{
		((Uint8 *)(sdl_screen_cursor->pixels))[i] = !(i & 2) ? 0x80 : 0xFF;
	}

	return 0;
}

errr SDL_DrawCursor(SDL_Surface *dst, SDL_Rect *dr)
{
	if (!dst || !dr || !sdl_screen_cursor) return -1;
	if (SDL_BlitSurface(sdl_screen_cursor, &sdl_screen_cursor_sr, dst, dr)) return -1;
	SDL_UpdateRect(dst, dr->x, dr->y, dr->w, dr->h);
	return 0;
}

#endif

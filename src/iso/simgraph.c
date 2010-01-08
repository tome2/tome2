/* simgraph.c
 *
 * Copyright (c) 2001 Hansjörg Malthaner
 * hansjoerg.malthaner@gmx.de
 *
 * This file is part of the Simugraph graphics engine.
 *
 *
 * This file may be copied and modified freely so long as the above credits,
 * this paragraph, and the below disclaimer of warranty are retained; no
 * financial profit is derived from said modification or copying; and all
 * licensing rights to any modifications are granted to the original author,
 * Hansjörg Malthaner.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */


/* simgraph.c
 * 
 * Versuch einer Graphic fuer Simulationsspiele
 * Hj. Malthaner, Aug. 1997
 *                                     
 * A try to create a graphics engine for simulation games.
 *
 * 3D, isometrische Darstellung        
 *
 * 3D, isometric display
 *
 *
 * 18.11.97 lineare Speicherung fuer Images -> hoehere Performance
 * 22.03.00 run längen Speicherung fuer Images -> hoehere Performance 
 * 15.08.00 dirty tile verwaltung fuer effizientere updates
 */

//#define DEBUG 1


#if defined(MSDOS) || defined(__MINGW32__)
#define USE_SOFTPOINTER
#endif

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "simgraph.h"
#include "simsys.h"                

extern unsigned int   class;
extern unsigned int   code;
extern int            mx,my;             /* es sind negative Koodinaten mgl */

#ifdef USE_SOFTPOINTER
static int softpointer = 261;
static int old_my = -1;              // die icon leiste muss neu gezeichnet werden wenn der
                                     // Mauszeiger darueber schwebt 
#endif

static unsigned char dr_fonttab[2048];  /* Unser Zeichensatz sitzt hier */

/*
 * pixel value type, RGB 555
 */
typedef unsigned short PIXVAL;


struct imd {
    int y;           // offset top
    int h;           // image height
    int len;         // data length in entities of PIXVAL size
    PIXVAL * data;   // iamge data
}; 
                                

static int anz_images = 0; 

static int disp_width = 640;
static int disp_height = 480;

/**
 * Tile size in pixels
 *
 * @author Hj. Malthaner
 */
static int tile_size = 32;

static struct imd *images = NULL;
static struct imd *images1 = NULL;
static struct imd *images2 = NULL;


static PIXVAL *textur = NULL;


static struct clip_dimension clip_rect;


// dirty tile management strcutures
            
#define DIRTY_TILE_SIZE 32
#define DIRTY_TILE_SHIFT 5

static char *tile_dirty = NULL;
static char *tile_dirty_old = NULL;

static int tiles_per_line = 0;
static int tile_lines = 0;
                                  

// colormap management structures

static unsigned char day_pal[256*3];
static unsigned char night_pal[256*3];
static unsigned char base_pal[256*3];

static int light_level = 0;
static int color_level = 1;


// ------------ procedure like defines --------------------


#define mark_tile_dirty(x,y) tile_dirty[(x) + (y)*tiles_per_line] = 1
#define is_tile_dirty(x,y) ((tile_dirty[(x) + (y)*tiles_per_line]) || (tile_dirty_old[(x) + (y)*tiles_per_line])  )

// ----------------- procedures ---------------------------
            

/**
 * Markiert ein Tile as schmutzig, ohne Clipping
 * @author Hj. Malthaner
 */
static void mark_rect_dirty_nc(int x1, int y1, int x2, int y2)
{      
    int x, y;
    
    // floor to tile size

    x1 >>= DIRTY_TILE_SHIFT;
    y1 >>= DIRTY_TILE_SHIFT;
    x2 >>= DIRTY_TILE_SHIFT;
    y2 >>= DIRTY_TILE_SHIFT;

    for(y=y1; y<=y2; y++) {
	for(x=x1; x<=x2; x++) {
	    mark_tile_dirty(x, y);
	}
    }    
}

/**
 * Markiert ein Tile as schmutzig, mit Clipping
 * @author Hj. Malthaner
 */
void mark_rect_dirty_wc(int x1, int y1, int x2, int y2)
{      
    if(x1 < 0) x1 = 0;
    if(x1 >= disp_width) x1 = disp_width-1;

    if(y1 < 0) y1 = 0;
    if(y1 >= disp_height) y1 = disp_height-1;

    if(x2 < 0) x2 = 0;
    if(x2 >= disp_width) x2 = disp_width-1;

    if(y2 < 0) y2 = 0;
    if(y2 >= disp_height) y2 = disp_height-1;

    mark_rect_dirty_nc(x1, y1, x2, y2);
}

/**
 * Clipped einen Wert in Intervall
 * @author Hj. Malthaner
 */
static int clip(int w, int u, int o)
{
    return w < u ? u : w > o ? o : w;
}


/**
 * Laedt den Font
 * @author Hj. Malthaner
 */
static void init_font()
{
    FILE *f = NULL;

    // suche in ./draw.fnt

    if(f==NULL ) {
	f=fopen("./draw.fnt","rb");
    }

    if(f==NULL) {
	printf("Cannot open draw.fnt!\n");
	exit(1);
    } else {
	int i;
	for(i=0;i<2048;i++) dr_fonttab[i]=getc(f);
	    fclose(f);
    }
}

/**
 * Laedt die Palette
 * @author Hj. Malthaner
 */
static int
load_palette(const char *fname, unsigned char *palette)
{
    FILE *file = fopen(fname,"rb");

    if(file) {

	int    x;
        int    anzahl=256;
	int    r,g,b;

        fscanf(file, "%d", &anzahl);

        for(x=0; x<anzahl; x++) {
            fscanf(file,"%d %d %d", &r, &g, &b);

	    palette[x*3+0] = r;
	    palette[x*3+1] = g;
	    palette[x*3+2] = b;
	}
	dr_setRGB8multi(0, anzahl, palette);

	fclose(file);                   

	return TRUE;
    } else {
	fprintf(stderr, "Error: can't open file '%s' for reading\n", fname);
	return FALSE;
    }
}


/**
 * Dims (darkens) a color. 
 * @return darkended color.
 * @author Hj. Malthaner
 */
static inline int darken(const int dunkel, const PIXVAL farbe)
{
    int r,g,b; 
     
    r = (farbe & 0x7C00) - (dunkel << 10);

    if(r < 0) {
	r = 0;
    }

    g = (farbe & 0x03E0) - (dunkel << 5);

    if(g < 0) {
	g = 0;
    }

    b = (farbe & 0x001F) - dunkel;
    
    if(b < 0) {
	b = 0;
    }

    return (r & 0x7C00) + (g & 0x03E0) + b;
}


int display_get_width()
{
    return disp_width;
}

int display_get_height()
{
    return disp_height;
}
 

/**
 * returns the currently used tile size in pixels
 *
 * @author Hj. Malthaner
 */
int display_get_tile_size()
{
    return tile_size;
}


/**
 * selects a tile size
 *                   
 * @param n 0 means 64x64 tiles, 1 are 32x32 tiles
 * @author Hj. Malthaner
 */
void display_select_tile_size(int n)
{
    switch(n) {
    case 0:
	tile_size = 64;
	images = images1;
	break;

    case 1:
	tile_size = 32;
	images = images2;
	break;

    default:
	tile_size = 64;
	images = images1;
    }        

    printf("Switching to tile size %d\n", tile_size);
}


/**
 * Holt Helligkeitseinstellungen
 * @author Hj. Malthaner
 */
int display_get_light()
{
    return light_level;
}


/**
 * Setzt Helligkeitseinstellungen
 * @author Hj. Malthaner
 */
void display_set_light(int new_light_level)
{
    unsigned char palette[256*3];
    const double ll = 1.0 - light_level/20.0;
    int i;

    light_level = new_light_level;

    for(i=0; i<256; i++) {
	const int n = i*3;
	palette[n+0] = clip(pow(base_pal[n+0]/255.0, ll)*255.0, 0, 255);
	palette[n+1] = clip(pow(base_pal[n+1]/255.0, ll)*255.0, 0, 255);
	palette[n+2] = clip(pow(base_pal[n+2]/255.0, ll)*255.0, 0, 255);
    }

    dr_setRGB8multi(0, 256, palette);
}

/**
 * Holt Farbeinstellungen
 * @author Hj. Malthaner
 */
int display_get_color()
{
    return color_level;
}


/**
 * Setzt Farbeinstellungen
 * @author Hj. Malthaner
 */
void display_set_color(int new_color_level)
{
    color_level = new_color_level;

    if(color_level < 0) {
	color_level = 0;
    }

    if(color_level > 3) {
	color_level = 3;
    }
    
    switch(color_level) {
    case 0:
	load_palette("./simud70.pal", day_pal);
	load_palette("./simun70.pal", night_pal);
	break;
    case 1:
	load_palette("./simud80.pal", day_pal);
	load_palette("./simun80.pal", night_pal);
	break;
    case 2:
	load_palette("./simud90.pal", day_pal);
	load_palette("./simun90.pal", night_pal);
	break;
    case 3:
	load_palette("./simud100.pal", day_pal);
	load_palette("./simun100.pal", night_pal);
	break;
    }                         

    memcpy(base_pal, day_pal, 256*3);
    display_set_light(display_get_light());
}


static int night_shift = -1;

static void calc_base_pal_from_night_shift(const int night)
{                                                
    const int day = 4 - night;
    int i;

    for(i=0; i<256; i++) {
	base_pal[i*3+0] = (day_pal[i*3+0] * day + night_pal[i*3+0] * night) >> 2;
	base_pal[i*3+1] = (day_pal[i*3+1] * day + night_pal[i*3+1] * night) >> 2;
	base_pal[i*3+2] = (day_pal[i*3+2] * day + night_pal[i*3+2] * night) >> 2;
    }
}

    
void display_day_night_shift(int night)
{
    if(night != night_shift) {
	night_shift = night;

	calc_base_pal_from_night_shift(night);	

	display_set_light(light_level);
	mark_rect_dirty_nc(0,0, disp_width-1, disp_height-1);
    }
}



/**
 * Setzt Farbeintrag
 * @author Hj. Malthaner
 */
void display_set_player_colors(const unsigned char *day, const unsigned char *night)
{
    memcpy(day_pal, day, 12);
    memcpy(night_pal, night, 12);

    calc_base_pal_from_night_shift(night_shift);	

    display_set_light(light_level);
    mark_rect_dirty_nc(0,0, disp_width-1, disp_height-1);
}


/**
 * Liest 32Bit wert Plattfromunabhängig
 * @author Hj. Malthaner
 */
static int fread_int(FILE *f)
{
    int i = 0;

    i += fgetc(f);
    i += fgetc(f) << 8;
    i += fgetc(f) << 16;
    i += fgetc(f) << 24;

    return i;
}


/**
 * Laedt daten.pak
 * @author Hj. Malthaner
 */
static struct imd * init_images(const char *filename)
{                                                   
    FILE *f = fopen(filename, "rb");                
    struct imd * images = NULL;

    if( f ) {
	int i;

	anz_images = fread_int(f);
	images = malloc(sizeof(struct imd)*anz_images);


	for(i=0; i<anz_images; i++) {
            images[i].y    = fread_int(f);
            images[i].h    = fread_int(f);
            images[i].len  = fread_int(f);

//	    printf("len = %d\n", images[i].len);

	    if(images[i].h > 0) {            
                images[i].data = malloc(images[i].len*sizeof(PIXVAL));
		fread(images[i].data, images[i].len*sizeof(PIXVAL), 1, f);

	    } else {
		images[i].data = NULL;
	    }          
	}

	fclose(f);
    } else {
	printf("Kann '%s' nicht lesen.\n", filename);
	exit(1);
    }

    return images;
}

/**
 * Holt Maus X-Position 
 * @author Hj. Malthaner
 */
int gib_maus_x()
{
    return mx;
}

/**
 * Holt Maus y-Position 
 * @author Hj. Malthaner
 */
int gib_maus_y()
{
    return my;
}

                     
/**
 * this sets width < 0 if the range is out of bounds
 * so check the value after calling and before displaying
 * @author Hj. Malthaner
 */
static int clip_wh(int *x, int *width, const int min_width, const int max_width)
{
    // doesnt check "wider than image" case

    if(*x < min_width) {
	const int xoff = min_width - (*x);

	*width += *x;
	*x = min_width;
	return xoff;
    } else if(*x + *width >= max_width) {
	*width = max_width - *x;
    }
    return 0;
} 


/**
 * Ermittelt Clipping Rechteck
 * @author Hj. Malthaner
 */
struct clip_dimension display_gib_clip_wh(void)
{
    return clip_rect;
}


/**
 * Setzt Clipping Rechteck
 * @author Hj. Malthaner
 */
void display_setze_clip_wh(int x, int y, int w, int h)
{
    clip_wh(&x, &w, 0, disp_width);
    clip_wh(&y, &h, 0, disp_height);

    clip_rect.x = x;
    clip_rect.y = y;
    clip_rect.w = w;
    clip_rect.h = h;    

    clip_rect.xx = x+w-1;
    clip_rect.yy = y+h-1;
}

// ----------------- basic painting procedures ----------------


/**
 * Kopiert Pixel von src nach dest
 * @author Hj. Malthaner
 */
static void pixcopy(PIXVAL *dest,
                    const PIXVAL *src,
                    int len)
{
    memcpy(dest, src, len*sizeof(PIXVAL));
}


/**
 * Zeichnet Bild mit Clipping
 * @author Hj. Malthaner
 */
static void
display_img_wc(const int n, const int xp, const int yp, const int dirty)
{       
    if(n >= 0 && n < anz_images) {

	int h = images[n].h;
	int y = yp + images[n].y;

        int yoff = clip_wh(&y, &h, 0, clip_rect.yy);

	if(h > 0) {
            const int width = disp_width;
            const PIXVAL *sp = images[n].data;
            PIXVAL *tp = textur + y*width;

	    if(dirty) {
		mark_rect_dirty_wc(xp, y, xp+tile_size-1, y+h+1);
	    }

	    // oben clippen
	    
	    while(yoff) {
		yoff --;
		do {
		    if(*(++sp)) {
			sp += *sp + 1;
		    }                        		    
		} while(*sp);
		sp ++;
	    }		    

	    do { // zeilen dekodieren
                int xpos = xp;

		// bild darstellen

		int runlen = *sp++;

		do { 
		    // wir starten mit einem clear run

		    xpos += runlen;               

		    // jetzt kommen farbige pixel
                    runlen = *sp++;

		    if(runlen) {  

			if(xpos >= 0 && runlen+xpos < width) {
//                            pixcopy(tp+xpos, sp, runlen);
			    memcpy(tp+xpos, sp, runlen*sizeof(PIXVAL));
                        } else if(xpos < 0) {
			    if(runlen+xpos > 0) {
//                                pixcopy(tp, sp-xpos, runlen+xpos);
				memcpy(tp, sp-xpos, (runlen+xpos)*sizeof(PIXVAL));
			    }
			} else if(width > xpos) {
//                            pixcopy(tp+xpos, sp, width-xpos);
                              memcpy(tp+xpos, sp, (width-xpos)*sizeof(PIXVAL));
			}
			sp += runlen;
			xpos += runlen;
			runlen = *sp ++;
		    }                        		    
		} while(runlen);
                           
		tp += width;
		
	    } while(--h > 0);
	}
    }
}

/**
 * Zeichnet Bild ohne Clipping
 * @author Hj. Malthaner
 */
static void
display_img_nc(const int n, const int xp, const int yp, const int dirty)
{                               
    if(n >= 0 && n < anz_images) {

	int h = images[n].h;

	if(h > 0) {
            const PIXVAL *sp = images[n].data;
            PIXVAL *tp = textur + (yp + images[n].y)*disp_width + xp;

	    if(dirty) {
		mark_rect_dirty_nc(xp, yp+images[n].y, xp+tile_size-1, yp+images[n].y+h-1);
	    }

	    do { // zeilen dekodieren

		// bild darstellen
		
		int runlen = *sp++;

		do { 

		    // wir starten mit einem clear run
		    tp += runlen;               

		    // jetzt kommen farbige pixel
		    runlen = *sp++;

		    if(runlen) {
//			pixcopy(tp, sp, runlen);
			memcpy(tp, sp, runlen*sizeof(PIXVAL));
		        sp += runlen; 
			tp += runlen;
			runlen = *sp++;
		    }
		} while(runlen);
                           
		tp += (disp_width-tile_size);
		
	    } while(--h);
	}
    }
}

/**
 * Zeichnet Bild
 * @author Hj. Malthaner
 */
void
display_img(const int n, const int xp, const int yp, const int dirty)
{
    if(xp>=0 && yp>=0 && xp < disp_width-tile_size-1 && yp < disp_height-tile_size-1) {
	display_img_nc(n, xp, yp, dirty);
    } else {
	if(xp>-tile_size && yp>-tile_size && xp < disp_width && yp < disp_height) {	
	    display_img_wc(n, xp, yp, dirty);                              
	}
    }
}



/**
 * Copies and shades colors
 * @param shade the amount to darken the color
 * @author Hj. Malthaner
 */
static void colorpixcpy(PIXVAL *dest, const PIXVAL *src, 
                        const PIXVAL * const end, 
                        const PIXVAL shade)
{   
    while(src < end) {
	*dest++ = darken(shade, *src++);
    }   
}


/**
 * Zeichnet Bild, ersetzt Spielerfarben
 * @author Hj. Malthaner
 */
static void
display_color_img_aux(const int n, const int xp, const int yp, const int color, const int dirty)
{                
    if(n >= 0 && n < anz_images) {

	int h = images[n].h;
	int y = yp + images[n].y;

        int yoff = clip_wh(&y, &h, 0, clip_rect.yy);

	if(h > 0) {
            const int width = disp_width;
            const PIXVAL *sp = images[n].data;
            PIXVAL *tp = textur + y*width;

//	    printf("textur = %p  tp = %p\n", textur, tp);

	    if(dirty) {
      		mark_rect_dirty_wc(xp, y+yoff, xp+tile_size-1, y+yoff+h-1);
	    }

	    // oben clippen

	    while(yoff) {
		yoff --;
		do {
		    if(*(++sp)) {
			sp += *sp + 1;
		    }                        		    
		} while(*sp);
	        sp ++;
	    }		    

	    do { // zeilen dekodieren
                int xpos = xp;

		// bild darstellen

		do { 
		    // wir starten mit einem clear run

		    xpos += *sp ++;               

		    // jetzt kommen farbige pixel

		    if(*sp) {
			const int runlen = *sp++;
			
			if(xpos >= 0 && runlen+xpos < width) {
			    colorpixcpy(tp+xpos, sp, sp+runlen, color);
			} else if(xpos < 0) {
			    if(runlen+xpos > 0) {
				colorpixcpy(tp, sp-xpos, sp+runlen, color);
			    }
			} else if(width > xpos) {
			    colorpixcpy(tp+xpos, sp, sp+width-xpos, color);
			}

			sp += runlen;
			xpos += runlen;
		    }                        		    
		} while(*sp);
                           
		tp += width;
	        sp ++;
		
	    } while(--h);
	}
    }
}


/**
 * Zeichnet Bild, ersetzt Farben
 * @author Hj. Malthaner
 */
void
display_color_img(const int n, const int xp, const int yp, const int color, const int dirty)
{
    // since the colors for player 0 are already right,
    // only use the expensive replacement routine for colored images
    // of other players

    // printf("color=%d\n", color);
                
    if(color) {
	display_color_img_aux(n, xp, yp, color, dirty);
    } else {
	display_img_wc(n, xp, yp, dirty);
    }
}

/**
 * Zeichnet ein Pixel
 * @author Hj. Malthaner
 */
void
display_pixel(int x, int y, const int color)
{   
    if(x >= clip_rect.x && x<=clip_rect.xx &&
       y >= clip_rect.y && y<clip_rect.yy) {

        PIXVAL * const p = textur + x + y*disp_width;
	*p = color;

	mark_tile_dirty(x >> DIRTY_TILE_SHIFT, y >> DIRTY_TILE_SHIFT);
    }
}


/**
 * Zeichnet einen Text, lowlevel Funktion
 * @author Hj. Malthaner
 */
static void
dr_textur_text(PIXVAL *textur,int x,int y,const char *txt, 
               const int chars, const int fgpen, const int dirty)
{
    int p;

    y+=4;                               /* Korektu amiga <-> pc */

    if(y < 0 || y+8 >= disp_height)
	return;                       /* out of clip */


    if(dirty) {
	mark_rect_dirty_nc(x, y, x+chars*8-1, y+8-1);
    }


    for(p=0; p<chars; p++) {      /* Zeichen fuer Zeichen ausgeben */
	int base=((unsigned char *)txt)[p] << 3;                 /* 8 Byte je Zeichen */
	const int end = base+8;
	int screen_pos = x + (p << 3) + y*disp_width;


	do {
	    const int c=dr_fonttab[base++];       /* Eine Zeile des Zeichens */
	    int b;

	    for(b=0; b<8; b++) {
		if(c & (128 >> b)) {
		    textur[screen_pos+b] = fgpen;
		} 
	    }
	    screen_pos += disp_width;
	}while(base < end);
    }
}


/**
 * Zeichnet Text, highlevel Funktion
 * @author Hj. Malthaner
 */
void
display_text(int x, int y, const char *txt, const int color, int dirty)
{                                        
    const int chars = strlen(txt);
    const int text_width = chars*8;

    if(y >= 8 && y < disp_height-12) {

	if(x >= 0  && x+text_width < disp_width) {
	    dr_textur_text(textur, x, y, txt, chars, color, dirty);
	} else {
	    if(x < 0 && x+text_width > 8) {       
		const int left_chars = (-x+7)/8;		

		dr_textur_text(textur, (x & 7), y, txt+left_chars, chars-left_chars, color, dirty);
            } else if(x > 0 && x < disp_width-7) {
                const int rest_chars = (disp_width-x-1) / 8;

		dr_textur_text(textur, x, y, txt, rest_chars, color, dirty);		
	    }
	}
    }
}
                                          
/**
 * Zeichnet gefuelltes Rechteck, ohne clipping
 * @author Hj. Malthaner
 */
void display_fb_internal(int xp, int yp, int w, int h, 
			 const int color, const int dirty,
			 int cL, int cR, int cT, int cB)
{
    clip_wh(&xp, &w, cL, cR);
    clip_wh(&yp, &h, cT, cB);

    if(w > 0 && h > 0) {
        PIXVAL *p = textur + xp + yp*disp_width;

	if(dirty) {
	    mark_rect_dirty_nc(xp, yp, xp+w-1, yp+h-1);
	}

	do {
	    memset(p, color, w*sizeof(PIXVAL));
            p += disp_width;
	} while(--h);
    }
}
 
void
display_fillbox_wh(int xp, int yp, int w, int h,
		   const int color, const int dirty)
{
  display_fb_internal(xp,yp,w,h,color,dirty,
		      0,disp_width-1,0,disp_height-1);
} 
void
display_fillbox_wh_clip(int xp, int yp, int w, int h,
			const int color, const int dirty)
{
  display_fb_internal(xp,yp,w,h,color,dirty,
		      clip_rect.x, clip_rect.xx, clip_rect.y, clip_rect.yy);
} 

/**
 * Zeichnet vertikale Linie
 * @author Hj. Malthaner
 */
void
display_vl_internal(const int xp, int yp, int h, const int color, int dirty,
		    int cL, int cR, int cT, int cB)
{
    clip_wh(&yp, &h, cT, cB);

    if(xp >= cL && xp <= cR && h > 0) {
        PIXVAL *p = textur + xp + yp*disp_width;

        if (dirty) {
	  mark_rect_dirty_nc(xp, yp, xp, yp+h-1);
	}
	    
	do {
	    *p = color;
            p += disp_width;
	} while(--h);           
    }
}

void
display_vline_wh(int xp, int yp, int h, const int color, const int dirty)
{
  display_vl_internal(xp,yp,h,color,dirty,
		      0,disp_width-1,0,disp_height-1);
} 

void
display_vline_wh_clip(int xp, int yp, int h, const int color, const int dirty)
{
  display_vl_internal(xp,yp,h,color,dirty,
		      clip_rect.x, clip_rect.xx, clip_rect.y, clip_rect.yy);
} 

/**
 * Zeichnet rohe Pixeldaten
 * @author Hj. Malthaner
 */
void
display_array_wh(int xp, int yp, int w, int h, const unsigned char *arr)
{            
    const int arr_w = w;

    clip_wh(&xp, &w, 0, disp_width);
    clip_wh(&yp, &h, 0, disp_height);

    if(w > 0 && h > 0) {
        PIXVAL *p = textur + xp + yp*disp_width;

	mark_rect_dirty_nc(xp, yp, xp+w-1, yp+h-1);
                      
	if(xp == 0) {
	    arr += arr_w - w;
	}

	do {
	    // FIXME!!!
	    memcpy(p, arr, w);
            p += disp_width;
	    arr += arr_w;
	} while(--h);
    }   
} 
     

// --------------- compound painting procedures ---------------


/**
 * Zeichnet schattiertes Rechteck
 * @author Hj. Malthaner
 */
void
display_ddd_box(int x1, int y1, int w, int h, int tl_color, int rd_color)
{
    display_fillbox_wh(x1, y1, w, 1, tl_color, TRUE);
    display_fillbox_wh(x1, y1+h-1, w, 1, rd_color, TRUE);

    h-=2;

    display_vline_wh(x1, y1+1, h, tl_color, TRUE);
    display_vline_wh(x1+w-1, y1+1, h, rd_color, TRUE);
}

/**
 * Zeichnet schattierten  Text
 * @author Hj. Malthaner
 */
void
display_ddd_text(int xpos, int ypos, int hgt, 
                 int ddd_farbe, int text_farbe,
                 const char *text, int dirty)
{
    const int len = strlen(text)*4;

    display_fillbox_wh(xpos-2-len,
                       ypos-hgt-6,
                       4 + len*2, 1,
                       ddd_farbe+1,
		       dirty);
    display_fillbox_wh(xpos-2-len,
                       ypos-hgt-5,
                       4 + len*2, 8,
                       ddd_farbe,
		       dirty);
    display_fillbox_wh(xpos-2-len,
                       ypos-hgt+3,
                       4 + len*2, 1,
                       ddd_farbe-1,
		       dirty);

    display_text(xpos - len, 
                 ypos-hgt-9,
                 text, 
		 text_farbe,
		 dirty);
}


/**
 * Zaehlt Vorkommen eines Buchstabens in einem String
 * @author Hj. Malthaner
 */
int
count_char(const char *str, const char c)
{
    int count = 0;

    while(*str) {
	count += (*str++ == c);
    }
    return count;
}

/**
 * Zeichnet einen mehrzeiligen Text
 * @author Hj. Malthaner
 */
void
display_multiline_text(int x, int y, const char *inbuf, int color)
{
    char tmp[4096];
    char *buf = tmp;
    char *next;                        
    int y_off = 0;           

    // be sure not to copy more than buffer size
    strncpy(buf, inbuf, 4095);    

    // always close with a 0 byte
    buf[4095] = 0;

    while( (*buf != 0) && (next = strchr(buf,'\n')) ) { 
	*next = 0;
	display_text(x,y+y_off, buf, color, TRUE);
	buf = next+1;
	y_off += LINESPACE;
    }                    
}


/**
 * Loescht den Bildschirm
 * @author Hj. Malthaner
 */
void
display_clear()
{
    memset(textur+32*disp_width, 32, disp_width*(disp_height-17-32));

    mark_rect_dirty_nc(0, 0, disp_width-1, disp_height-1);
}


#if 0
void display_flush_buffer()
{
    int x, y;
    char * tmp;

#ifdef USE_SOFTPOINTER
    display_img(softpointer, mx, my, TRUE);
    old_my = my;
#endif

#ifdef DEBUG
    // just for debugging
    int tile_count = 0;
#endif

    for(y=0; y<tile_lines; y++) {
#ifdef DEBUG

	for(x=0; x<tiles_per_line; x++) {
	    if(is_tile_dirty(x, y)) {
		display_fillbox_wh(x << DIRTY_TILE_SHIFT,
                                   y << DIRTY_TILE_SHIFT,
				   DIRTY_TILE_SIZE/4,
				   DIRTY_TILE_SIZE/4,
				   3,
				   FALSE);
				   


		dr_textur(x << DIRTY_TILE_SHIFT,
                          y << DIRTY_TILE_SHIFT,
			  DIRTY_TILE_SIZE,
			  DIRTY_TILE_SIZE);

		tile_count ++;
	    } else {
		display_fillbox_wh(x << DIRTY_TILE_SHIFT,
                                   y << DIRTY_TILE_SHIFT,
				   DIRTY_TILE_SIZE/4,
				   DIRTY_TILE_SIZE/4,
				   0,
				   FALSE);
				   


		dr_textur(x << DIRTY_TILE_SHIFT,
                          y << DIRTY_TILE_SHIFT,
			  DIRTY_TILE_SIZE,
			  DIRTY_TILE_SIZE);

	    }
	}
#else
	x = 0;

	do {
	    if(is_tile_dirty(x, y)) {
		const int xl = x;		
                do {
		    x++;
		} while(x < tiles_per_line && is_tile_dirty(x, y));

		dr_textur(xl << DIRTY_TILE_SHIFT,
                          y << DIRTY_TILE_SHIFT,
			  (x-xl)<<DIRTY_TILE_SHIFT,
			  DIRTY_TILE_SIZE);

	    }
	    x++;
	} while(x < tiles_per_line);
#endif
    }

#ifdef DEBUG                              
//    printf("%d von %d tiles wurden gezeichnet\n", tile_count, tile_lines*tiles_per_line);
#endif

    dr_flush();

    // swap tile buffers
    tmp = tile_dirty_old;
    tile_dirty_old = tile_dirty;

    tile_dirty = tmp;
    memset(tile_dirty, 0, tile_lines*tiles_per_line);
}
#endif /* 0 */

void display_flush_buffer()
{             
    dr_textur(0, 0, disp_width, disp_height);
}

/**
 * Bewegt Mauszeiger
 * @author Hj. Malthaner
 */
void display_move_pointer(int dx, int dy)
{
    move_pointer(dx, dy);
}


/**
 * Schaltet Mauszeiger sichtbar/unsichtbar
 * @author Hj. Malthaner
 */
void display_show_pointer(int yesno)
{
#ifdef USE_SOFTPOINTER
    if(yesno) {
	softpointer = 261;
    } else {
	softpointer = 52;
    }
#else
    show_pointer(yesno);
#endif
}

/**
 * unbenutzt ?
 * @author Hj. Malthaner
 */ 
void      
my_save_exit()
{
    dr_os_close();
}
                     

/**
 * Inits. Grafikmodul
 * @author Hj. Malthaner
 */
int
simgraph_init(int width, int height)
{
    int parameter[2];
    int ok;

    dr_os_init(0, parameter);

    ok = dr_os_open(width, height);

    if(ok) {                

	disp_width = dr_get_width();
	disp_height = dr_get_height();

        textur = dr_textur_init();

	// not needed for iso-band
	// init_font(".drawrc");


//	display_set_color(1);


	images1 = init_images("daten.pak");

	images2 = init_images("daten2.pak");
                         
	display_select_tile_size(0);

        printf("Init. done.\n");   

//        dr_use_color(rp, SCHWARZ);
//        dr_fillbox_wh(rp, 0, 0, disp_width, WIN_disp_height);

    } else {
	puts("Error  : can't open window!");
        exit(-1);
    }


    // allocate dirty tile flags
    tiles_per_line = (disp_width + DIRTY_TILE_SIZE - 1) / DIRTY_TILE_SIZE;
    tile_lines = (disp_height + DIRTY_TILE_SIZE - 1) / DIRTY_TILE_SIZE;

    tile_dirty = malloc( tile_lines*tiles_per_line );
    tile_dirty_old = malloc( tile_lines*tiles_per_line );

    memset(tile_dirty, 1, tile_lines*tiles_per_line);
    memset(tile_dirty_old, 1, tile_lines*tiles_per_line);

    display_setze_clip_wh(0, 0, disp_width, disp_height);

    return TRUE;
}

/**
 * Prueft ob das Grafikmodul schon init. wurde
 * @author Hj. Malthaner
 */
int is_display_init()
{
    return textur != NULL;
}

/**
 * Schliest das Grafikmodul
 * @author Hj. Malthaner
 */
int
simgraph_exit()
{
    free(tile_dirty);
    free(tile_dirty_old);


    return dr_os_close();
}


/**
 * Laedt Einstellungen
 * @author Hj. Malthaner
 */
void display_laden(FILE* file)
{
    int i,r,g,b;

    unsigned char day[12];
    unsigned char night[12];

    fscanf(file, "%d %d %d\n", &light_level, &color_level, &night_shift);

    display_set_light(light_level);
    display_set_color(color_level);

    for(i=0; i<4; i++) {
	fscanf(file, "%d %d %d\n", &r, &g, &b);
	day[i*3+0] = r;
	day[i*3+1] = g;
	day[i*3+2] = b;

	fscanf(file, "%d %d %d\n", &r, &g, &b);
	night[i*3+0] = r;
	night[i*3+1] = g;
	night[i*3+2] = b;
    }

    display_set_player_colors(day, night);
}


/**
 * Speichert Einstellungen
 * @author Hj. Malthaner
 */
void display_speichern(FILE* file)
{
    int i;
    fprintf(file, "%d %d %d\n", light_level, color_level, night_shift);

    for(i=0; i<4; i++) {
	fprintf(file, "%d %d %d\n", day_pal[i*3+0], day_pal[i*3+1], day_pal[i*3+2]);
	fprintf(file, "%d %d %d\n", night_pal[i*3+0], night_pal[i*3+1], night_pal[i*3+2]);
    }
}


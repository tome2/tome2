/* simgraph.h
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

/* simgraph.h
 * 
 * Versuch einer Graphic fuer Simulationsspiele
 * Hj. Malthaner, Aug. 1997
 *                                     
 *
 * 3D, isometrische Darstellung        
 *
 */

#ifndef simgraph_h
#define simgraph_h

#ifdef __cplusplus
extern "C" { 
#endif
      

#ifndef TRUE
#define TRUE 1
#define FALSE 0
#endif

#define LINESPACE 11


struct clip_dimension {
    int x, xx, w, y, yy, h;
};


// helper macros

// save the current clipping and set a new one
#define PUSH_CLIP(x,y,w,h) \
{\
const struct clip_dimension p_cr = display_gib_clip_wh(); \
display_setze_clip_wh(x, y, w, h);

// restore a saved clipping rect
#define POP_CLIP() \
display_setze_clip_wh(p_cr.x, p_cr.y, p_cr.w, p_cr.h); \
}


// function prototypes

int simgraph_init(int width, int height);
int is_display_init();
int simgraph_exit();

int gib_maus_x();
int gib_maus_y();

void mark_rect_dirty_wc(int x1, int y1, int x2, int y2);


/**
 * returns the currently used tile size in pixels
 *
 * @author Hj. Malthaner
 */
int display_get_tile_size();


/**
 * selects a tile size
 *                   
 * @param n 0 means 64x64 tiles, 1 are 32x32 tiles
 * @author Hj. Malthaner
 */
void display_select_tile_size(int n);



int display_get_width();
int display_get_height();


int  display_get_light();
void display_set_light(int new_light_level);

int display_get_color();
void display_set_color(int new_color_level);

void display_day_night_shift(int night);

//void display_set_rgb(int n, int r, int g, int b);
void display_set_player_colors(const unsigned char *day, const unsigned char *night);

void display_img(const int n, const int xp, const int yp, const int dirty);
void display_color_img(const int n, const int xp, const int yp, const int color, const int dirty);
void display_fillbox_wh(int xp, int yp, int w, int h, int color, int dirty);
void display_fillbox_wh_clip(int xp, int yp, int w, int h, int color, int d);
void display_vline_wh(const int xp, int yp, int h, const int color, int dirty);
void display_vline_wh_clip(const int xp, int yp, int h, const int c, int d);
void display_clear();

void display_flush_buffer();

void display_move_pointer(int dx, int dy);
void display_show_pointer(int yesno);

  
void display_pixel(int x, int y, int color);

void display_ddd_text(int xpos, int ypos, int hgt, 
                      int ddd_farbe, int text_farbe,
                      const char *text, int dirty);

void display_text(int x, int y, const char *txt, const int color, int dirty);
void display_array_wh(int xp, int yp, int w, int h, const unsigned char *arr);
void display_ddd_box(int x1, int y1, int w, int h, int tl_color, int rd_color);

// compound painting routines

int count_char(const char *str, const char c);
void display_multiline_text(int x, int y, const char *inbuf, int color);

void zeige_banner(void);

void display_setze_clip_wh(int x, int y, int w, int h);
struct clip_dimension display_gib_clip_wh(void);


#ifdef __cplusplus
}
#endif

#endif

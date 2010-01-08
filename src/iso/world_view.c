/* world_view.c
 *
 * Copyright (c) 2001, 2002 Hansjörg Malthaner
 * hansjoerg.malthaner@gmx.de
 *
 * This file is part of the Simugraph<->Angband adaption code.
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

#include <stdio.h>
#include "simgraph.h" 
#include "world_view.h" 
#include "world_adaptor.h" 

//#include "walls4.h"
#include "walls9.h"

#include "hackdef.h"


#undef MIN
#undef MAX


#include "../angband.h"
#include "../defines.h"


/* from isov-x11.c */
extern unsigned char **iso_ap;
extern unsigned char **iso_cp;
extern unsigned char **iso_atp;
extern unsigned char **iso_ctp;
# ifdef USE_EGO_GRAPHICS
extern unsigned char **iso_aep;
extern unsigned char **iso_cep;
# endif /* USE_EGO_GRAPHICS */


#ifdef USE_SMALL_ISO_HACK

static bool is_valid_position(int x, int y) {
  return x >= 0 && y >= 0 && x<cur_wid && y<cur_hgt;
}                             

static bool is_valid_to_show(int x, int y) {
  return is_valid_position(x, y);
}

static int get_feat_nc(int x, int y)
{
  /* this is for Angabdn 2.9.1
     return cave_feat[y][x];     
  */
  
  /* Pernangband 4.1.5 */
  cave_type *c_ptr;
  
  /* Get the cave */
  c_ptr = &cave[y][x];
  
  
  /* Feature code */
  return c_ptr->feat;
}


static int get_feat_wc(int x, int y)
{
  if(is_valid_position(x,y)) {
    return get_feat_nc(x,y);
  } else {
    return FEAT_PERM_SOLID;
  }
}


static int get_info_nc(int x, int y)
{
  /* this is for Angabdn 2.9.1
     return cave_info[y][x];     
  */
  
  /* Pernangband 4.1.5 */
  cave_type *c_ptr;
  
  /* Get the cave */
  c_ptr = &cave[y][x];
  
  
  /* Info code */
  return c_ptr->info;
}


static int get_info_wc(int x, int y)
{
  if(is_valid_position(x,y)) {
    return get_info_nc(x,y);
  } else {
    return 0;
  }
}


static bool is_door(int feat)
{
  return 
    (feat == FEAT_OPEN) ||
    (feat == FEAT_BROKEN) ||
    (feat == FEAT_SECRET) ||
    (feat >= FEAT_DOOR_HEAD &&  feat <= FEAT_DOOR_TAIL) ||
    (feat == FEAT_SHOP);
}


static bool is_wall(int feat)
{
  return 
    (feat >= FEAT_MAGMA &&  feat <= FEAT_PERM_SOLID) || 
    (feat >= FEAT_TREES &&  feat <= FEAT_SANDWALL_K) ||
    (feat >= FEAT_QUEST1 && feat <= FEAT_QUEST4);
}


static int is_wall_or_door(int x, int y)
{           
  const int feat = get_feat_wc(x, y);
  
  return is_door(feat) || is_wall(feat);
}


static bool is_lit(int x, int y)
{
  const int info = get_info_wc(x, y);
  
  return (info & CAVE_GLOW) && (info & CAVE_VIEW) && (info & CAVE_MARK);
}


static bool is_torch_lit(int x, int y)
{
  const int info = get_info_wc(x, y);
  
  return (info & CAVE_PLIT) && (info & CAVE_VIEW) && (info & CAVE_MARK);
}


static bool is_only_torch_lit(int x, int y)
{
  return is_torch_lit(x, y) & !is_lit(x, y);
}


static bool is_blind()
{
  // PernAngband variant
  return p_ptr->blind;
}


/**
 * Calculates distance from player to point (x,y).
 * Uses the distance guess from cave.c
 * @author Hj. Malthaner
 */
static int get_distance_to_player(int x, int y)
{                                
  return distance(y, x, p_ptr->py, p_ptr->px);
}

#else


static int iso_access(unsigned char **field, int x, int y)
{
  if(x >= 0 && y >= 0 && x<80 && y<24) {
    return field[y][x] & 0x7f;
  } else {
    return 0;
  }                       
}

static int is_wall_or_door(int x, int y)
{           
  const int atp = iso_access(iso_atp, x, y);
  const int ctp = iso_access(iso_ctp, x, y);
  
  return (atp == 0x01 && (ctp >= 0x70 && ctp <= 0x75)) ||
    (atp == 0x00  && (ctp =='+' || ctp == '\'')) ;
}

#endif


/**
 * Draw ground of location i,j onto screen position xpos,ypos
 * @author Hj. Malthaner
 */
void display_boden(int x, int y, int xpos, int ypos)
{
  // unused in Iso-Angband
}


static int check_wall(int x, int y) 
{
  
  int o = 0;

#if 0
  o |= (is_wall_or_door(x-1, y-1)) << 0;
  o |= (is_wall_or_door(x  , y-1)) << 1;
  o |= (is_wall_or_door(x+1, y-1)) << 2;
  o |= (is_wall_or_door(x-1, y  )) << 3;
  o |= (is_wall_or_door(x+1, y  )) << 4;
  o |= (is_wall_or_door(x-1, y+1)) << 5;
  o |= (is_wall_or_door(x  , y+1)) << 6;
  o |= (is_wall_or_door(x+1, y+1)) << 7;
  
  
  return wall_table[o];
#endif
  // jue: new algorithm as suggested by James Andrewartha
  //
  o |= (is_wall_or_door(x-1, y-1) 
          && is_wall_or_door(x, y-1)
          && is_wall_or_door(x-1,y))  << 0;

  o |= (is_wall_or_door(x  , y-1))    << 1;
  
  o |= (is_wall_or_door(x+1, y-1)
	  && is_wall_or_door(x, y-1)
	  && is_wall_or_door(x+1, y)) << 2;
  
  o |= (is_wall_or_door(x-1, y  ))    << 3;
  
  o |= (is_wall_or_door(x+1, y  ))    << 4;
  
  o |= (is_wall_or_door(x-1, y+1)
	  && is_wall_or_door(x-1, y)
	  && is_wall_or_door(x, y+1)) << 5;
  
  o |= (is_wall_or_door(x  , y+1))    << 6;
  
  o |= (is_wall_or_door(x+1, y+1)
	  && is_wall_or_door(x+1, y)
	  && is_wall_or_door(x, y+1)) << 7;
  
  return o;
}


static int door_direction(int x, int y)
{                 
  if(is_wall_or_door(x-1, y) && is_wall_or_door(x+1, y)) {
    return 1;
  } else {
    return 0;
  }
}

static int calc_nc(int c, int a)
{
  return ((a & 0x7F) << 7) + (c & 0x7F);
}


/**
 * Draw objects of location i,j onto screen position xpos,ypos
 * @author Hj. Malthaner
 */
void display_dinge(int x, int y, int xpos, int ypos)
{
  const int grid = get_grid();

  int feat_nc = -1;
  int obj_nc = -1;
#ifdef USE_EGO_GRAPHICS
  int ego_nc = -1;
#endif
  
  int shade;
  int i;

  // relative to view position
//  int xoff = x - p_ptr->px + (SCREEN_WID/2 + 13);
//  int xoff = x - p_ptr->px + (SCREEN_WID/2 + 13);
//
//  new (jue):
//  look at cave.c, panel_col_of and the places it's used...
  int xoff = x - panel_col_min + COL_MAP;
  int yoff = y - panel_row_prt;

  // try to use output of the term package
  if(xoff >= 0 && yoff >= 1 && xoff < 80 && yoff < 23) {
    // floor, walls
    
    const int c = iso_cp[yoff][xoff];
    const int a = iso_ap[yoff][xoff];
    
    // object/monster
    const int tc = iso_ctp[yoff][xoff];
    const int ta = iso_atp[yoff][xoff];

#ifdef USE_EGO_GRAPHICS
    // transparent overlay 
    const int ec = iso_cep[yoff][xoff];
    const int ea = iso_aep[yoff][xoff];
    
    ego_nc = calc_nc(ec, ea);
#endif
    feat_nc = calc_nc(tc, ta);
    obj_nc = calc_nc(c, a);

  } else {
    // outside 80x24 view, try to read map

    if(is_valid_to_show(x,y)) {
      
      byte a, ta;
      char c, tc;
#ifdef USE_EGO_GRAPHICS
      byte ea;
      char ec;

      map_info(y, x, &a, &c, &ta, &tc, &ea, &ec);
      ego_nc = calc_nc(ec, ea);
#else
      map_info(y, x, &a, &c, &ta, &tc);
#endif
      feat_nc = calc_nc(tc, ta);
      obj_nc = calc_nc(c, a);
    } 

  }



  shade = 1;
      
  if(is_lit(x, y)) {
    shade = 2;
  } else if(is_torch_lit(x, y)) {
    shade = 0;
  }

  
  // did we get some data ?
  if(feat_nc != -1) {
    
    // check shading
    //	printf("%d %d -> %d\n", x, y, shade);
    
    if(feat_nc >= 240 &&  feat_nc <= 245) {
      // old shading
      //	    const int shade = (feat_nc-240);
      const int set_base = 240+shade*9;
      const int bits = check_wall(x, y);
      
      
      display_img(316+(shade%3), xpos, ypos, TRUE);
      
      if(bits) {
	for(i=0; i<4; i++) {
	  if(bits & (1 << i)) {
	    display_img(set_base+i, xpos, ypos, TRUE);
	  }
	}                         
	
	display_img(set_base+8, xpos, ypos, TRUE);
	
	for(i=4; i<8; i++) {
	  if(bits & (1 << i)) {
	    display_img(set_base+i, xpos, ypos, TRUE);
	  }
	}                         
	
      } else {
	// a pillar 
	display_img(set_base+8, xpos, ypos, TRUE);
      }
      
      // fields with walls never contain anything else
      // so we can quit now
      return;
      
    } else if(feat_nc == 0x27) {
      // open doors   
      display_img(138+door_direction(x,y), xpos, ypos, TRUE);
      
      
    } else if(feat_nc == 0x2B) {
      // closed doors
      display_img(136+door_direction(x,y), xpos, ypos, TRUE);
      
    } else if(feat_nc == 0x8C ||	    // a between gate
	      feat_nc == 0x20 ||	    // empty square
	      feat_nc == 0x16E ||	    // brick roof
	      feat_nc == 0x16F ||	    // brick roof top
	      feat_nc == 0x176 ||	    // grass roof
	      feat_nc == 0x177 ||	    // grass roof chimney
	      feat_nc == 0x17E ||	    // grass roof top
	      (feat_nc >= 0x31 && feat_nc <= 0x39) || // shop with number
	      (feat_nc >= 0x576 && feat_nc <= 0x57D) || // trap
	      feat_nc == 0x3E ||	    // down stairs
	      feat_nc == 0x3C	    // up stairs
    ) {
      // this features should not be shaded
      
      display_img(feat_nc, xpos, ypos, TRUE);
      
    } else if(feat_nc > 2){             
      // this features should be shaded  
          
      // floor
      // known grids get shaded floors
      if(is_only_torch_lit(x,y)) {
	display_color_img(feat_nc+2, 
			  xpos, ypos, 
			  get_distance_to_player(x, y) << 1,
			  TRUE);
      } else {
	display_img(feat_nc+shade, xpos, ypos, TRUE);
      }               
    }
    
    //	printf("%d a=%x c=%x (%c)\n", nc, a, c, c);
  
    /* Hajo: display a complete grid if the user wants to */
    if(grid == 2) {
	display_img(6, xpos, ypos, TRUE);
    }


#ifdef USE_EGO_GRAPHICS   
    if( ego_nc != feat_nc &&
	ego_nc > 2 &&
	ego_nc != 0x27 &&
	ego_nc != 0x2B ) {

	display_img(ego_nc, xpos, ypos, TRUE);
    }
#endif
    
    if( obj_nc != feat_nc &&
	obj_nc > 2 &&
	obj_nc != 0x27 &&
	obj_nc != 0x2B ) {

	/* Hajo: display a grid below items/monsters if the user wants to */
	if(grid == 1) {
	    display_img(5, xpos, ypos, TRUE);
	}
	
	display_img(obj_nc, xpos, ypos, TRUE);

    }
  } else {
    
    // outside of dungeon
    display_img(32, xpos, ypos, TRUE);
  }  

  /* display cursor ? */
  if( 
      high_x >= 1 &&
      high_y >= 1) {
      if(xoff == high_x && 
	  yoff == high_y) {
        display_img(7, xpos, ypos, TRUE);

	/* only draw once, wait until next request */
	high_x = high_y = -1;
    }
  }
}


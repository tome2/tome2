#pragma once

#include "h-basic.h"

bool_ new_player_spot(int branch);
void add_level_generator(cptr name, bool_ (*generator)());
bool_ level_generate_dungeon();
bool_ generate_fracave(int y0, int x0,int xsize,int ysize,int cutoff,bool_ light,bool_ room);
void generate_hmap(int y0, int x0,int xsiz,int ysiz,int grd,int roug,int cutoff);
bool_ room_alloc(int x,int y,bool_ crowded,int by0,int bx0,int *xx,int *yy);
void generate_cave(void);
void build_rectangle(int y1, int x1, int y2, int x2, int feat, int info);

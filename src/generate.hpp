#pragma once

#include "h-basic.hpp"

bool new_player_spot(int branch);
void add_level_generator(const char *name, bool (*generator)());
bool level_generate_dungeon();
bool generate_fracave(int y0, int x0, int xsize, int ysize, int cutoff, bool light, bool room);
void generate_hmap(int y0, int x0,int xsiz,int ysiz,int grd,int roug,int cutoff);
bool room_alloc(int x,int y,bool crowded,int by0,int bx0,int *xx,int *yy);
void generate_cave();
void build_rectangle(int y1, int x1, int y2, int x2, int feat, int info);

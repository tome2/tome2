#pragma once

#include "h-basic.h"

void print_desc_aux(cptr txt, int y, int x);
void save_savefile_names(void);
bool_ begin_screen(void);
void player_birth(void);
void roll_player_hp();

extern bool_ no_begin_screen;

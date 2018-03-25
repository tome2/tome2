#pragma once

#include "h-basic.h"

void print_desc_aux(const char *txt, int y, int x);
void save_savefile_names();
bool_ begin_screen();
void player_birth();
void roll_player_hp();

extern bool_ no_begin_screen;

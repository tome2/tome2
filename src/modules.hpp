#pragma once

#include "h-basic.h"

bool_ select_module();
bool_ module_savefile_loadable(cptr savefile_mod);
void tome_intro();
void theme_intro();
s16b *theme_race_status(int r_idx);
void init_hooks_module();
int find_module(cptr name);
bool_ private_check_user_directory(cptr dirpath);
extern cptr force_module;

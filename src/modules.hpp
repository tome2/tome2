#pragma once

#include "h-basic.h"

extern bool_ select_module(void);
extern bool_ module_savefile_loadable(cptr savefile_mod);
extern void tome_intro();
extern void theme_intro();
extern s16b *theme_race_status(int r_idx);
extern void init_hooks_module();
extern int find_module(cptr name);
extern bool_ private_check_user_directory(cptr dirpath);
extern cptr force_module;

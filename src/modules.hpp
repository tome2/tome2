#pragma once

#include "h-basic.h"
#include <string>

bool_ select_module();
bool module_savefile_loadable(std::string const &savefile_mod);
void tome_intro();
void theme_intro();
s16b *theme_race_status(int r_idx);
void init_hooks_module();
int find_module(const char *name);
bool_ private_check_user_directory(const char *dirpath);
extern const char *force_module;

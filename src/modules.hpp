#pragma once

#include "h-basic.hpp"
#include "program_args.hpp"

#include <string>

bool select_module(program_args const &);
bool module_savefile_loadable(std::string const &savefile_mod);
void tome_intro();
void theme_intro();
s16b *theme_race_status(int r_idx);
void init_hooks_module();
int find_module(const char *name);
bool private_check_user_directory(const char *dirpath);

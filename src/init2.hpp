#pragma once

#include "h-basic.h"
#include "program_args.hpp"

void init_file_paths(char *path);
void init_angband(program_args const &);
void init_corruptions();
void create_stores_stock(int t);
errr init_v_info();
extern s16b error_idx;
extern s16b error_line;

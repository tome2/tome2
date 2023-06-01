#pragma once

#include "h-basic.hpp"
#include "program_args.hpp"

#include <string>

bool load_dungeon(std::string const &ext);
bool load_player(program_args const &);
void save_dungeon();
bool save_player();

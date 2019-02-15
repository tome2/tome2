#pragma once

#include "h-basic.hpp"

#include <string>

void quest_library_init_hook();
std::string quest_library_describe();
void quest_library_building(bool *paid, bool *recreate);
void initialize_bookable_spells();

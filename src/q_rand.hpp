#pragma once

#include "h-basic.hpp"

#include <string>

void initialize_random_quests(int n);
bool_ is_randhero(int level);
void quest_random_init_hook();
std::string quest_random_describe();

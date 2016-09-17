#pragma once

#include "h-basic.h"

#include <string>

void initialize_random_quests(int n);
bool_ is_randhero(int level);
bool_ quest_random_init_hook();
std::string quest_random_describe();

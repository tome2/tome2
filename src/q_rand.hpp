#pragma once

#include "angband.h"

void initialize_random_quests(int n);
bool_ is_randhero(int level);
bool_ quest_random_init_hook(int q_idx);
bool_ quest_random_describe(FILE *fff);

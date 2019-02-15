#pragma once

#include "h-basic.hpp"

void inc_piety(int god, s32b amt);
void abandon_god(int god);
int wisdom_scale(int max);
int find_god(const char *name);
void follow_god(int god, bool silent);
bool god_enabled(struct deity_type *deity);
deity_type *god_at(byte god_idx);
bool show_god_info();
bool praying_to(int god);

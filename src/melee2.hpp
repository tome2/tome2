#pragma once

#include "h-basic.h"
#include "monster_type_fwd.hpp"

extern int monst_spell_monst_spell;
bool mon_take_hit_mon(int s_idx, int m_idx, int dam, const char *note);
void mon_handle_fear(monster_type *m_ptr, int dam, bool *fear);
int check_hit2(int power, int level, int ac);
void process_monsters();
void curse_equipment(int chance, int heavy_chance);
void curse_equipment_dg(int chance, int heavy_chance);

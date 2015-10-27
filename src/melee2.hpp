#pragma once

#include "h-basic.h"
#include "monster_type_fwd.hpp"

extern int monst_spell_monst_spell;
extern bool_ mon_take_hit_mon(int s_idx, int m_idx, int dam, bool_ *fear, cptr note);
extern void mon_handle_fear(monster_type *m_ptr, int dam, bool_ *fear);
extern int check_hit2(int power, int level, int ac);
extern void process_monsters(void);
extern void curse_equipment(int chance, int heavy_chance);
extern void curse_equipment_dg(int chance, int heavy_chance);

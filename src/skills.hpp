#pragma once

#include "h-basic.h"

#include <cstddef>

/* Skill functions */
void dump_skills(FILE *fff);
s16b find_skill(cptr name);
s16b find_skill_i(cptr name);
s16b get_skill(int skill);
s16b get_skill_scale(int skill, u32b scale);
void do_cmd_skill(void);
void do_cmd_activate_skill(void);
cptr get_melee_name();
s16b get_melee_skills(void);
s16b get_melee_skill(void);
bool_ forbid_gloves(void);
bool_ forbid_non_blessed(void);
void compute_skills(s32b *v, s32b *m, std::size_t i);
void select_default_melee(void);
void do_get_new_skill(void);
void init_skill(s32b value, s32b mod, std::size_t i);
s16b find_ability(cptr name);
void dump_abilities(FILE *fff);
void do_cmd_ability(void);
void apply_level_abilities(int level);
void recalc_skills(bool_ init);

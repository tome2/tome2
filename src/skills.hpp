#pragma once

#include "h-basic.h"

#include <cstddef>
#include <string>

/* Skill functions */
void dump_skills(FILE *fff);
s16b find_skill(const char *name);
s16b find_skill_i(std::string const &name);
s16b find_skill_i(const char *name);
s16b get_skill(int skill);
s16b get_skill_scale(int skill, u32b scale);
void do_cmd_skill();
void do_cmd_activate_skill();
const char *get_melee_name();
s16b get_melee_skills();
s16b get_melee_skill();
bool_ forbid_gloves();
bool_ forbid_non_blessed();
void compute_skills(s32b *v, s32b *m, std::size_t i);
void select_default_melee();
void do_get_new_skill();
void init_skill(s32b value, s32b mod, std::size_t i);
s16b find_ability(std::string const &name);
s16b find_ability(const char *name);
void dump_abilities(FILE *fff);
void do_cmd_ability();
void apply_level_abilities(int level);
void recalc_skills(bool_ init);

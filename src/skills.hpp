#pragma once

#include "h-basic.h"

/* Skill functions */
extern void dump_skills(FILE *fff);
extern s16b find_skill(cptr name);
extern s16b find_skill_i(cptr name);
extern s16b get_skill(int skill);
extern s16b get_skill_scale(int skill, u32b scale);
extern void do_cmd_skill(void);
extern void do_cmd_activate_skill(void);
extern cptr get_melee_name();
extern s16b get_melee_skills(void);
extern s16b get_melee_skill(void);
extern bool_ forbid_gloves(void);
extern bool_ forbid_non_blessed(void);
extern void compute_skills(s32b *v, s32b *m, std::size_t i);
extern void select_default_melee(void);
extern void do_get_new_skill(void);
extern void init_skill(s32b value, s32b mod, std::size_t i);
extern s16b find_ability(cptr name);
extern void dump_abilities(FILE *fff);
extern void do_cmd_ability(void);
extern void apply_level_abilities(int level);
extern void recalc_skills(bool_ init);

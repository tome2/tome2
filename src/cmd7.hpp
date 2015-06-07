#pragma once

#include "h-basic.h"
#include "rune_spell_fwd.hpp"
#include "object_type_fwd.hpp"

extern void do_cmd_pray(void);
extern void do_cmd_create_boulder(void);
extern int rune_exec(rune_spell *spell, int cost);
extern void necro_info(char *p, int power);
extern void mindcraft_info(char *p, int power);
extern void symbiotic_info(char *p, int power);
extern void mimic_info(char *p, int power);
extern void do_cmd_summoner(void);
extern void do_cmd_mindcraft(void);
extern void do_cmd_mimic(void);
extern void use_ability_blade(void);
extern int alchemist_learn_object(object_type *o_ptr);
extern void do_cmd_alchemist(void);
extern void do_cmd_beastmaster(void);
extern void do_cmd_powermage(void);
extern void do_cmd_possessor(void);
extern void do_cmd_archer(void);
extern void do_cmd_set_piercing(void);
extern void do_cmd_necromancer(void);
extern void do_cmd_unbeliever(void);
extern void do_cmd_runecrafter(void);
extern void do_cmd_symbiotic(void);
extern s32b sroot(s32b n);
extern int clamp_failure_chance(int chance, int minfail);

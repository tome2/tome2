#pragma once

#include "h-basic.h"
#include "object_type_fwd.hpp"

void do_cmd_pray(void);
void do_cmd_create_boulder(void);
void necro_info(char *p, int power);
void mindcraft_info(char *p, int power);
void symbiotic_info(char *p, int power);
void mimic_info(char *p, int power);
void do_cmd_summoner(void);
void do_cmd_mindcraft(void);
void do_cmd_mimic(void);
void use_ability_blade(void);
void do_cmd_beastmaster(void);
void do_cmd_powermage(void);
void do_cmd_possessor(void);
void do_cmd_archer(void);
void do_cmd_set_piercing(void);
void do_cmd_necromancer(void);
void do_cmd_unbeliever(void);
void do_cmd_symbiotic(void);
s32b sroot(s32b n);
int clamp_failure_chance(int chance, int minfail);

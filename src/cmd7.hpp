#pragma once

#include "h-basic.h"
#include "object_type_fwd.hpp"

void do_cmd_pray();
void do_cmd_create_boulder();
void necro_info(char *p, int power);
void mindcraft_info(char *p, int power);
void symbiotic_info(char *p, int power);
void mimic_info(char *p, int power);
void do_cmd_summoner();
void do_cmd_mindcraft();
void do_cmd_mimic();
void use_ability_blade();
void do_cmd_beastmaster();
void do_cmd_powermage();
void do_cmd_possessor();
void do_cmd_archer();
void do_cmd_set_piercing();
void do_cmd_necromancer();
void do_cmd_unbeliever();
void do_cmd_symbiotic();
s32b sroot(s32b n);
int clamp_failure_chance(int chance, int minfail);

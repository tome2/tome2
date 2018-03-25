#pragma once

#include "h-basic.h"
#include "monster_type_fwd.hpp"

void dump_companions(FILE *outfile);
void do_cmd_companion();
bool do_control_reconnect();
bool do_control_drop();
bool do_control_magic();
bool do_control_pickup();
bool do_control_inven();
bool do_control_walk();
bool can_create_companion();
void ai_deincarnate(int m_idx);
void ai_possessor(int m_idx, int o_idx);
bool ai_multiply(int m_idx);
bool change_side(monster_type *m_ptr);
int is_friend(monster_type *m_ptr);
bool is_enemy(monster_type *m_ptr, monster_type *t_ptr);

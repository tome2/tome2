#pragma once

#include "h-basic.h"
#include "monster_type_fwd.hpp"

void dump_companions(FILE *outfile);
void do_cmd_companion();
bool_ do_control_reconnect();
bool_ do_control_drop();
bool_ do_control_magic();
bool_ do_control_pickup();
bool_ do_control_inven();
bool_ do_control_walk();
bool_ can_create_companion();
void ai_deincarnate(int m_idx);
bool_ ai_possessor(int m_idx, int o_idx);
bool_ ai_multiply(int m_idx);
bool_ change_side(monster_type *m_ptr);
int is_friend(monster_type *m_ptr);
bool_ is_enemy(monster_type *m_ptr, monster_type *t_ptr);

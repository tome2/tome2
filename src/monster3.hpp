#pragma once

#include "h-basic.h"
#include "monster_type_fwd.hpp"

void dump_companions(FILE *outfile);
void do_cmd_companion(void);
bool_ do_control_reconnect(void);
bool_ do_control_drop(void);
bool_ do_control_magic(void);
bool_ do_control_pickup(void);
bool_ do_control_inven(void);
bool_ do_control_walk(void);
bool_ can_create_companion(void);
void ai_deincarnate(int m_idx);
bool_ ai_possessor(int m_idx, int o_idx);
bool_ ai_multiply(int m_idx);
bool_ change_side(monster_type *m_ptr);
int is_friend(monster_type *m_ptr);
bool_ is_enemy(monster_type *m_ptr, monster_type *t_ptr);

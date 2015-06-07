#pragma once

#include "h-basic.h"
#include "monster_type_fwd.hpp"

extern void dump_companions(FILE *outfile);
extern void do_cmd_companion(void);
extern bool_ do_control_reconnect(void);
extern bool_ do_control_drop(void);
extern bool_ do_control_magic(void);
extern bool_ do_control_pickup(void);
extern bool_ do_control_inven(void);
extern bool_ do_control_walk(void);
extern bool_ can_create_companion(void);
extern void ai_deincarnate(int m_idx);
extern bool_ ai_possessor(int m_idx, int o_idx);
extern bool_ ai_multiply(int m_idx);
extern bool_ change_side(monster_type *m_ptr);
extern int is_friend(monster_type *m_ptr);
extern bool_ is_enemy(monster_type *m_ptr, monster_type *t_ptr);

#pragma once

#include "h-basic.h"
#include "object_type_fwd.hpp"
#include <vector>

std::vector<s16b> show_monster_inven(int m_idx);
int breakage_chance(object_type *o_ptr);
void do_cmd_go_up(void);
void do_cmd_go_down(void);
void do_cmd_search(void);
void do_cmd_toggle_search(void);
void do_cmd_open(void);
void do_cmd_close(void);
void do_cmd_chat(void);
void do_cmd_give(void);
void do_cmd_tunnel(void);
void do_cmd_bash(void);
void do_cmd_alter(void);
void do_cmd_spike(void);
void do_cmd_walk(int pickup);
void do_cmd_stay(int pickup);
void do_cmd_run(void);
void do_cmd_rest(void);
int get_shooter_mult(object_type *o_ptr);
void do_cmd_fire(void);
void do_cmd_throw(void);
void do_cmd_boomerang(void);
void do_cmd_immovable_special(void);
void fetch(int dir, int wgt, bool_ require_los);
void do_cmd_sacrifice(void);
void do_cmd_steal(void);

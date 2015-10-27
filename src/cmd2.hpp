#pragma once

#include "h-basic.h"
#include "object_type_fwd.hpp"
#include <vector>

extern std::vector<s16b> show_monster_inven(int m_idx);
extern int breakage_chance(object_type *o_ptr);
extern void do_cmd_go_up(void);
extern void do_cmd_go_down(void);
extern void do_cmd_search(void);
extern void do_cmd_toggle_search(void);
extern void do_cmd_open(void);
extern void do_cmd_close(void);
extern void do_cmd_chat(void);
extern void do_cmd_give(void);
extern void do_cmd_tunnel(void);
extern void do_cmd_disarm(void);
extern void do_cmd_bash(void);
extern void do_cmd_alter(void);
extern void do_cmd_spike(void);
extern void do_cmd_walk(int pickup, bool_ disarm);
extern void do_cmd_stay(int pickup);
extern void do_cmd_run(void);
extern void do_cmd_rest(void);
extern int get_shooter_mult(object_type *o_ptr);
extern void do_cmd_fire(void);
extern void do_cmd_throw(void);
extern void do_cmd_boomerang(void);
extern void do_cmd_immovable_special(void);
extern void fetch(int dir, int wgt, bool_ require_los);
extern void do_cmd_sacrifice(void);
extern void do_cmd_create_artifact(object_type *q_ptr);
extern void do_cmd_steal(void);

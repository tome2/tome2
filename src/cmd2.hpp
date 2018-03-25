#pragma once

#include "h-basic.h"
#include "object_type_fwd.hpp"
#include <vector>

std::vector<s16b> show_monster_inven(int m_idx);
int breakage_chance(object_type *o_ptr);
void do_cmd_go_up();
void do_cmd_go_down();
void do_cmd_search();
void do_cmd_toggle_search();
void do_cmd_open();
void do_cmd_close();
void do_cmd_chat();
void do_cmd_give();
void do_cmd_tunnel();
void do_cmd_bash();
void do_cmd_alter();
void do_cmd_spike();
void do_cmd_walk(int pickup);
void do_cmd_stay(int pickup);
void do_cmd_run();
void do_cmd_rest();
int get_shooter_mult(object_type *o_ptr);
void do_cmd_fire();
void do_cmd_throw();
void do_cmd_boomerang();
void do_cmd_immovable_special();
void do_cmd_sacrifice();
void do_cmd_steal();

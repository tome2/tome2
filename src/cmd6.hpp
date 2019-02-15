#pragma once

#include "h-basic.hpp"
#include "object_type_fwd.hpp"

void set_stick_mode(object_type *o_ptr);
void unset_stick_mode();
void do_cmd_eat_food();
void do_cmd_quaff_potion();
void do_cmd_read_scroll();
void do_cmd_aim_wand();
void do_cmd_use_staff();
void do_cmd_zap_rod();
const char *activation_aux(object_type *o_ptr, bool desc, int item);
void do_cmd_activate();
void do_cmd_cut_corpse();
void do_cmd_cure_meat();
void do_cmd_drink_fountain();

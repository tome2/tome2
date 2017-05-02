#pragma once

#include "h-basic.h"
#include "object_type_fwd.hpp"

void set_stick_mode(object_type *o_ptr);
void unset_stick_mode(void);
void do_cmd_eat_food(void);
void do_cmd_quaff_potion(void);
void do_cmd_read_scroll(void);
void do_cmd_aim_wand(void);
void do_cmd_use_staff(void);
void do_cmd_zap_rod(void);
const char *activation_aux(object_type *o_ptr, bool_ desc, int item);
void do_cmd_activate(void);
void do_cmd_cut_corpse(void);
void do_cmd_cure_meat(void);
void do_cmd_drink_fountain(void);

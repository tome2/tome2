#pragma once

#include "angband.h"

extern void set_stick_mode(object_type *o_ptr);
extern void unset_stick_mode(void);
extern void do_cmd_eat_food(void);
extern void do_cmd_quaff_potion(void);
extern void do_cmd_read_scroll(void);
extern void do_cmd_aim_wand(void);
extern void do_cmd_use_staff(void);
extern void do_cmd_zap_rod(void);
extern const char *activation_aux(object_type *o_ptr, bool_ desc, int item);
extern void do_cmd_activate(void);
extern void do_cmd_cut_corpse(void);
extern void do_cmd_cure_meat(void);
extern void do_cmd_drink_fountain(void);

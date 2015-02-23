#pragma once

#include "angband.h"

extern bool_ player_activate_trap_type(s16b y, s16b x, object_type *i_ptr, s16b item);
extern void player_activate_door_trap(s16b y, s16b x);
extern void place_trap(int y, int x);
extern void place_trap_leveled(int y, int x, int lev);
extern void place_trap_object(object_type *o_ptr);
extern void wiz_place_trap(int y, int x, int idx);
extern void do_cmd_set_trap(void);
extern bool_ mon_hit_trap(int);

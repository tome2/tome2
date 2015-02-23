#pragma once

#include "angband.h"

extern void inc_piety(int god, s32b amt);
extern void abandon_god(int god);
extern int wisdom_scale(int max);
extern int find_god(cptr name);
extern void follow_god(int god, bool_ silent);
extern bool_ god_enabled(struct deity_type *deity);
extern deity_type *god_at(byte god_idx);
extern bool_ show_god_info();

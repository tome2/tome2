#pragma once

#include "h-basic.h"
#include "object_flag_set.hpp"

#include <string>

extern void fix_message(void);
extern void apply_flags(object_flag_set const &f, s16b pval, s16b tval, s16b to_h, s16b to_d, s16b to_a);
extern int luck(int min, int max);
extern int weight_limit(void);
extern bool_ calc_powers_silent;
extern void cnv_stat(int i, char *out_val);
extern s16b modify_stat_value(int value, int amount);
extern void calc_hitpoints(void);
extern void notice_stuff(void);
extern void update_stuff(void);
extern void redraw_stuff(void);
extern void window_stuff(void);
extern void handle_stuff(void);
extern bool_ monk_heavy_armor(void);
extern void calc_bonuses(bool_ silent);
extern void gain_fate(byte fate);
extern std::string fate_desc(int fate);
extern std::string dump_fates();
extern bool race_flags1_p(u32b flags1_mask);
extern bool race_flags2_p(u32b flags2_mask);

#pragma once

#include "h-basic.hpp"
#include "object_flag_set.hpp"
#include "player_race_flag_set.hpp"

#include <string>

void fix_message();
void apply_flags(object_flag_set const &f, s16b pval, s16b tval, s16b to_h, s16b to_d, s16b to_a);
int luck(int min, int max);
int weight_limit();
extern bool_ calc_powers_silent;
void cnv_stat(int i, char *out_val);
s16b modify_stat_value(int value, int amount);
void calc_hitpoints();
void notice_stuff();
void update_stuff();
void redraw_stuff();
void window_stuff();
void handle_stuff();
bool_ monk_heavy_armor();
void calc_bonuses(bool_ silent);
void gain_fate(byte fate);
std::string fate_desc(int fate);
std::string dump_fates();
bool race_flags_p(player_race_flag_set const &flags_mask);

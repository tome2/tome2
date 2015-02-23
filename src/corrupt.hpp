#include "angband.h"

extern void gain_random_corruption();
extern void dump_corruptions(FILE *OutFile, bool_ color, bool_ header);
extern void lose_corruption();
extern bool_ player_has_corruption(int corruption_idx);
extern void player_gain_corruption(int corruption_idx);
extern s16b get_corruption_power(int corruption_idx);

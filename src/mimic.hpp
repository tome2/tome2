#include "h-basic.hpp"

s16b resolve_mimic_name(const char *name);
s16b find_random_mimic_shape(byte level, bool_ limit);
const char *get_mimic_name(s16b mf_idx);
const char *get_mimic_object_name(s16b mf_idx);
byte get_mimic_level(s16b mf_idx);
s32b get_mimic_random_duration(s16b mf_idx);
byte calc_mimic();
void calc_mimic_power();

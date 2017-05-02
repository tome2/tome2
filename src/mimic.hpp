#include "h-basic.h"

s16b resolve_mimic_name(cptr name);
s16b find_random_mimic_shape(byte level, bool_ limit);
cptr get_mimic_name(s16b mf_idx);
cptr get_mimic_object_name(s16b mf_idx);
byte get_mimic_level(s16b mf_idx);
s32b get_mimic_random_duration(s16b mf_idx);
byte calc_mimic();
void calc_mimic_power();

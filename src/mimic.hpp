#include "h-basic.h"

extern s16b resolve_mimic_name(cptr name);
extern s16b find_random_mimic_shape(byte level, bool_ limit);
extern cptr get_mimic_name(s16b mf_idx);
extern cptr get_mimic_object_name(s16b mf_idx);
extern byte get_mimic_level(s16b mf_idx);
extern s32b get_mimic_random_duration(s16b mf_idx);
extern byte calc_mimic();
extern void calc_mimic_power();

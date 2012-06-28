#ifndef H_eeb941c7_1a44_405a_8db3_ba14732c5b94
#define H_eeb941c7_1a44_405a_8db3_ba14732c5b94

#include "h-type.h"

#ifdef __cplusplus
extern "C" {
#endif

void quark_init();
cptr quark_str(s16b num);
s16b quark_add(cptr str);

#ifdef __cplusplus
} // extern "C"
#endif

#endif

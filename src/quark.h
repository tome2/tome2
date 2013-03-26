#pragma once

#include "h-basic.h"

#ifdef __cplusplus
extern "C" {
#endif

void quark_init();
cptr quark_str(s16b num);
s16b quark_add(cptr str);

#ifdef __cplusplus
} // extern "C"
#endif

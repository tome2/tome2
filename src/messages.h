#pragma once

#include "h-basic.h"

#ifdef __cplusplus
extern "C" {
#endif

void message_init();
s16b message_num();
cptr message_str(int age);
byte message_color(int age);
byte message_type(int age);
void message_add(cptr msg, byte color);

#ifdef __cplusplus
} // extern "C"
#endif

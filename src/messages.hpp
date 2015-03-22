#pragma once

#include "h-basic.h"

void message_init();
s16b message_num();
cptr message_str(int age);
byte message_color(int age);
void message_add(cptr msg, byte color);

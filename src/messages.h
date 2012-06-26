#ifndef H_f6dac2dc_0449_4764_9942_1c1fe7a70bc4
#define H_f6dac2dc_0449_4764_9942_1c1fe7a70bc4

#include "h-type.h"

void message_init();
s16b message_num();
cptr message_str(int age);
byte message_color(int age);
byte message_type(int age);
void message_add(cptr msg, byte color);

#endif

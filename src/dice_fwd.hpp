#pragma once

#include "h-basic.h"

typedef struct dice_type dice_type;
struct dice_type;

void dice_init(dice_type *dice, long base, long num, long sides);
bool_ dice_parse(dice_type *dice, const char *s);
void dice_parse_checked(dice_type *dice, const char *s);
long dice_roll(dice_type *dice);
void dice_print(dice_type *dice, char *buf);

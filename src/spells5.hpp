#pragma once

#include "h-basic.h"

void school_spells_init();
struct spell_type *spell_at(s32b index);
s16b get_random_spell(s16b random_type, int lev);
s16b get_random_stick(byte tval, int level);

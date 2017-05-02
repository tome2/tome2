#pragma once

#include "h-basic.h"
#include "spell_type_fwd.hpp"
#include "timer_type_fwd.hpp"

/** Calculate spell failure rate for a device, i.e. a wand or staff. */
s32b spell_chance_device(spell_type *spell_ptr);

/** Calculate spell failure rate for a spell book. */
s32b spell_chance_book(s32b s);


s32b lua_get_level(struct spell_type *spell, s32b lvl, s32b max, s32b min, s32b bonus);
int get_mana(s32b s);
s32b get_power(s32b s);
s32b get_level(s32b s, s32b max);
s32b get_level_s(int sp, int max);
void get_level_school(struct spell_type *spell, s32b max, s32b min, s32b *level, bool_ *na);

extern s32b get_level_max_stick;
extern s32b get_level_use_stick;

void get_map_size(const char *name, int *ysize, int *xsize);
void load_map(const char *name, int *y, int *x);

void increase_mana(int delta);

extern timer_type *TIMER_AGGRAVATE_EVIL;

void timer_aggravate_evil_enable();
void timer_aggravate_evil_callback();

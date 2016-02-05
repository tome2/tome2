#pragma once

#include "h-basic.h"
#include "spell_type_fwd.hpp"
#include "timer_type_fwd.hpp"

/** Calculate spell failure rate for a device, i.e. a wand or staff. */
extern s32b spell_chance_device(spell_type *spell_ptr);

/** Calculate spell failure rate for a spell book. */
extern s32b spell_chance_book(s32b s);


extern s32b lua_get_level(struct spell_type *spell, s32b lvl, s32b max, s32b min, s32b bonus);
extern int get_mana(s32b s);
extern s32b get_power(s32b s);
extern s32b get_level(s32b s, s32b max);
extern s32b get_level_s(int sp, int max);
extern void get_level_school(struct spell_type *spell, s32b max, s32b min, s32b *level, bool_ *na);

extern s32b get_level_max_stick;
extern s32b get_level_use_stick;

extern void get_map_size(const char *name, int *ysize, int *xsize);
extern void load_map(const char *name, int *y, int *x);

extern char *lua_input_box(cptr title, int max);
extern char lua_msg_box(cptr title);

extern void increase_mana(int delta);

extern timer_type *TIMER_AGGRAVATE_EVIL;

void timer_aggravate_evil_enable();
void timer_aggravate_evil_callback();

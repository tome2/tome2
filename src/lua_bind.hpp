#pragma once

#include "h-basic.h"

/** Calculate spell failure rate for a device, i.e. a wand or staff. */
extern s32b spell_chance_device(s32b s);

/** Calculate spell failure rate for a spell book. */
extern s32b spell_chance_book(s32b s);

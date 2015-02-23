#pragma once

#include "h-basic.h"
#include "spell_type_fwd.hpp"

/** Calculate spell failure rate for a device, i.e. a wand or staff. */
extern s32b spell_chance_device(spell_type *spell_ptr);

/** Calculate spell failure rate for a spell book. */
extern s32b spell_chance_book(s32b s);

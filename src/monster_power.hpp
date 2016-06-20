#pragma once

#include "monster_power_fwd.hpp"

#include "h-basic.h"

/**
 * Monster powers that players can use via e.g. Symbiosis.
 */
struct monster_power
{
	u32b    monster_spell_index;
	cptr    name;           /* Name of it */
	int     mana;           /* Mana needed */
	bool_    great;          /* Need the use of great spells */
};

#pragma once

#include "monster_power_fwd.hpp"

#include "h-basic.hpp"

/**
 * Monster powers that players can use via e.g. Symbiosis.
 */
struct monster_power
{
	u32b monster_spell_index;
	const char *name;           /* Name of it */
	int mana;           /* Mana needed */
	bool great;          /* Need the use of great spells */
};

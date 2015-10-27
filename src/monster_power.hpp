#pragma once

#include "h-basic.h"

/**
 * Monster powers that players can use via e.g. Symbiosis.
 */
struct monster_power
{
	u32b    power;          /* Power RF?_xxx */
	cptr    name;           /* Name of it */
	int     mana;           /* Mana needed */
	bool_    great;          /* Need the use of great spells */
};

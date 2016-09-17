#pragma once

#include "h-basic.h"

struct store_item
{
	/**
	 * Legal item kinds; if > 10000, designates (TVAL - 10000) and any SVAL.
	 * Otherwise designates an entry in k_info.txt.
	 */
	s16b kind = 0;

	/**
	 * Percentage chance of generation if the entry is chosen.
	 */
	s16b chance = 0;

};

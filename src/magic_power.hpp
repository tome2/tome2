#pragma once

#include "h-basic.h"

/**
 * Powers, used by Mindcrafters and Necromancers
 */
struct magic_power
{
	int min_lev;
	int mana_cost;
	int fail;
	const char *name;
	const char *desc;
};

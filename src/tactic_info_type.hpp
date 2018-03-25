#pragma once

#include "h-basic.h"

/**
 * Tactics descriptor.
 */
struct tactic_info_type
{
	s16b to_hit;
	s16b to_dam;
	s16b to_ac;
	s16b to_stealth;
	s16b to_saving;
	const char *name;
};

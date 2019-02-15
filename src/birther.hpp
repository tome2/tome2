#pragma once

#include "h-basic.h"

#include <array>

/**
 * Player information during the birth process.
 */
struct birther
{
	s16b race = 0;
	s16b rmod = 0;
	s16b pclass = 0;
	s16b spec = 0;

	byte quests = 0;

	byte god = 0;
	s32b grace = 0;

	s32b au = 0;

	std::array<s16b, 6> stat { };
	s16b luck = 0;

	bool quick_ok = false;
};

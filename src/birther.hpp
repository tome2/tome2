#pragma once

#include "h-basic.h"

#include <string>
#include <vector>

/**
 * Player information during the birth process.
 */
struct birther
{
	s16b race;
	s16b rmod;
	s16b pclass;
	s16b spec;

	byte quests;

	byte god;
	s32b grace;
	s32b god_favor;

	s32b au;

	s16b stat[6];
	s16b luck;

	bool_ quick_ok;
};

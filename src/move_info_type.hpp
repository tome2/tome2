#pragma once

#include "h-basic.hpp"

/**
 * Movement typse
 */
struct move_info_type
{
	s16b to_speed;
	s16b to_search;
	s16b to_stealth;
	s16b to_percep;
	const char *name;
};

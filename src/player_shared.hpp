#pragma once

#include "h-basic.hpp"
#include "skills_defs.hpp"
#include <array>
#include <vector>

struct player_shared
{
	std::array<s16b, 6> adj { };                       /* Stat modifiers */

	s16b mhp = 0;                                      /* Hit-dice adjustment */
	s16b exp = 0;                                      /* Experience factor */

	std::vector<s16b> powers;                          /* Powers */
};

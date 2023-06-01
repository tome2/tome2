#pragma once

#include "h-basic.hpp"

#include <array>
#include <string>

/**
 * Store/building actions.
 */
struct store_action_type
{
	std::string name;               /* Name */

	std::array<s16b, 3> costs { };  /* Costs for liked people */
	char letter = '\0';             /* Action letter */
	char letter_aux = '\0';         /* Action letter */
	s16b action = 0;                /* Action code */
	s16b action_restr = 0;          /* Action restriction */
};

#pragma once

#include "h-basic.h"
#include "store_type_fwd.hpp"

#include <vector>

/**
 * Town descriptor.
 */
struct town_type
{
	cptr name = nullptr;

	u32b seed = 0;                           /* Seed for RNG */

	std::vector<store_type> store;           /* The stores [max_st_idx] */

	byte flags = 0;                          /* Town flags */

	bool_ stocked = FALSE;                   /* Is the town actualy stocked ? */

	bool_ destroyed = FALSE;                 /* Is the town destroyed? */
};

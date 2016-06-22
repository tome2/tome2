#pragma once

#include "h-basic.h"
#include "store_type_fwd.hpp"

/**
 * Town descriptor.
 */
struct town_type
{
	cptr name = nullptr;

	u32b seed = 0;                           /* Seed for RNG */

	store_type *store = nullptr;             /* The stores [max_st_idx] */
	byte numstores = 0;

	byte flags = 0;                          /* Town flags */

	bool_ stocked = FALSE;                   /* Is the town actualy stocked ? */

	bool_ destroyed = FALSE;                 /* Is the town destroyed? */
};

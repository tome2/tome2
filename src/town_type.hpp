#pragma once

#include "h-basic.h"
#include "store_type_fwd.hpp"

/**
 * Town descriptor.
 */
struct town_type
{
	cptr name;
	u32b seed; /* Seed for RNG */
	store_type *store;      /* The stores [max_st_idx] */
	byte numstores;

	byte flags;             /* Town flags */
	/* Left this for the sake of compatibility */
	bool_ stocked;           /* Is the town actualy stocked ? */

	bool_ destroyed;         /* Is the town destroyed? */
};

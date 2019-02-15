#pragma once

#include "h-basic.hpp"
#include "seed.hpp"
#include "store_type_fwd.hpp"

#include <vector>

/**
 * Town descriptor.
 */
struct town_type
{
	const char *name = nullptr;

	seed_t seed = seed_t::system();  /* Seed for RNG */

	std::vector<store_type> store;           /* The stores [max_st_idx] */

	byte flags = 0;                          /* Town flags */

	bool stocked = false;                   /* Is the town actualy stocked ? */

	bool destroyed = false;                 /* Is the town destroyed? */
};

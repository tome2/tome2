#pragma once

#include "h-basic.hpp"
#include "object_type_fwd.hpp"

#include <vector>

/**
 * A store, with an owner, various state flags, a current stock
 * of items, and a table of items that are often purchased.
 */
struct store_type
{
	u16b st_idx = 0;

	/**
	 * Owner index
	 */
	u16b owner = 0;

	/**
	 * Closed until this turn.
	 */
	s32b store_open = 0;

	/**
	 * Last visited on this turn.
	 */
	s32b last_visit = 0;

	/**
	 * Stock: Total size of array
	 */
	u16b stock_size = 0;

	/**
	 * Stock: Actual stock items
	 */
	std::vector<object_type> stock;
};

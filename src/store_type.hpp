#pragma once

#include "h-basic.h"
#include "object_type.hpp"

/**
 * A store, with an owner, various state flags, a current stock
 * of items, and a table of items that are often purchased.
 */
struct store_type
{
	u16b st_idx;

	/**
	 * Owner index
	 */
	u16b owner;

	/**
	 * Closed until this turn.
	 */
	s32b store_open;

	/**
	 * Last visited on this turn.
	 */
	s32b last_visit;

	/**
	 * Stock: Number of entries.
	 */
	byte stock_num;

	/**
	 * Stock: Total size of array
	 */
	s16b stock_size;

	/**
	 * Stock: Actual stock items
	 */
	object_type *stock;
};

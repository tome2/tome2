#pragma once

#include "h-basic.h"

#include <boost/variant.hpp>

struct store_item_filter_by_k_idx
{
	s16b k_idx = -1;
};

struct store_item_filter_by_tval
{
	s16b tval = -1;
};

using store_item_filter_t = boost::variant<store_item_filter_by_k_idx, store_item_filter_by_tval>;

struct store_item
{
	/**
	 * Filter for the store items.
	 */
	store_item_filter_t filter;

	/**
	 * Percentage chance of generation if the entry is chosen.
	 */
	s16b chance = 0;

	/**
	 * Create a store item based on k_info index.
	 */
	static store_item k_idx(s16b idx, s16b chance)
	{
		store_item_filter_by_k_idx k;
		k.k_idx = idx;

		store_item i;
		i.chance = chance;
		i.filter = k;
		return i;
	}

	/**
	 * Create a store item based on TVAL.
	 */
	static store_item tval(s16b tval, s16b chance)
	{
		store_item_filter_by_tval k;
		k.tval = tval;

		store_item i;
		i.chance = chance;
		i.filter = k;
		return i;
	}

};

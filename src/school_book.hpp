#pragma once

#include "h-basic.hpp"

#include <vector>

/**
 * School book.
 */
struct school_book {
	/**
	 * Indexes of all the spells in the book.
	 */
	std::vector<s32b> spell_idxs;
};

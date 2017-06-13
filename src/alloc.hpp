#pragma once

#include "h-basic.h"
#include "alloc_entry.hpp"

#include <vector>

/**
 * Allocations of object kinds and monster races.
 */
struct Alloc {

	/*
	 * The entries in the "kind allocator table"
	 */
	std::vector<alloc_entry> kind_table;

	/*
	 * The flag to tell if kind_table contains valid entries
	 * for normal (i.e. kind_is_legal) object allocation
	 */
	bool kind_table_valid = false;

	/*
	 * The entries in the "race allocator table"
	 */
	std::vector<alloc_entry> race_table;

};

#pragma once

#include <array>
#include <string>

#include "h-basic.hpp"

/*
 * Store owner descriptor.
 */
struct owner_type
{
	/**
	 * Name
	 */
	std::string name;

	/**
	 * Purse limit
	 */
	s16b max_cost = 0;

	/**
	 * Inflation
	 */
	s16b inflation = 0;

	/**
	 * Liked/hated races.
	 */
	std::array<std::array<u32b, 2>, 2> races { };

	/**
	 * Liked/hated classes
	 */
	std::array<std::array<u32b, 2>, 2> classes { };

	/**
	 * Costs for liked people
	 */
	std::array<s16b, 3> costs { };
};

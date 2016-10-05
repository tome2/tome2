#pragma once

#include <string>

#include "h-basic.h"

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
	u32b races[2][2] { };

	/**
	 * Liked/hated classes
	 */
	u32b classes[2][2] { };

	/**
	 * Costs for liked people
	 */
	s16b costs[3] { };
};

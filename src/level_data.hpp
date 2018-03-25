#pragma once

#include "dungeon_flag_set.hpp"

#include <boost/optional.hpp>
#include <string>
#include <unordered_map>

/**
 * Read-only game data for individual levels of a dungeon.
 */
struct level_data
{
	/**
	 * Target dungeon number for side dungeon, if any.
	 */
	int branch = 0;

	/**
	 * Dungeon number for parent dungeon, if any. Filled in by post_d_info().
	 */
	int fbranch = 0;

	/**
	 * Dungeon depth for parent dungeon, if any. Filled in by post_d_info().
	 */
	int flevel = 0;

	/**
	 * Dungeon flags.
	 */
	dungeon_flag_set flags;

	/**
	 * Save file extension to use for this level, if any.
	 */
	boost::optional<std::string> save_extension;

	/**
	 * Get map name for special levels.
	 */
	boost::optional<std::string> map_name;

	/**
	 * Short name for special levels, replaces
	 * the regular depth display.
	 */
	boost::optional<std::string> name;

	/**
	 * One-line description for special levels,
	 * replaces the level feeling.
	 */
	boost::optional<std::string> description;

};

#pragma once

#include "dungeon_flag_set.hpp"

#include <boost/optional.hpp>
#include <string>

/**
 * Get the level description, for special levels.
 */
boost::optional<std::string> get_level_description();

/**
 * Get the level name, for special levels.
 */
boost::optional<std::string> get_dungeon_name();

/**
 * Get the map name, for special levels.
 */
boost::optional<std::string> get_dungeon_map_name();

/**
 * Get the dungeon save file extension, for special levels.
 */
boost::optional<std::string> get_dungeon_save_extension();

/**
 * Get dungeon flags, for special levels. Returns an empty
 * set of flags for non-special levels.
 */
dungeon_flag_set get_level_flags();

/**
 * Get target dungeon to which this level branches, if any.
 * Returns 0 when the dungeon level does not branch.
 */
int get_branch();

/**
 * Get parent dungeon of this dungeon, if any.
 * Returns 0 when the dungeon was not entered via a branch.
 */
int get_fbranch();

/**
 * Get the target depth of the parent dungeon, if any.
 * Returns 0 when the dungeon has no parent dungeon.
 */
int get_flevel();

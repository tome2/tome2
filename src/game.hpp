#pragma once

#include "game_fwd.hpp"
#include "grid.hpp"
#include "h-basic.h"
#include "player_defs.hpp"
#include "wilderness_map.hpp"

/**
 * All structures for the game itself.
 */
struct Game {

	/**
	 * Wilderness map
	 */
	grid<wilderness_map> wilderness;

	/**
	 * Player's un-adjusted HP at every level.
	 * Stored to avoid shenanigans with draininging levels
	 * and restoring them back, &c.
	 */
	std::array<s16b, PY_MAX_LEVEL> player_hp { };

};

/**
 * Game instance
 */
extern Game *game;

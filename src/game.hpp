#pragma once

#include "game_fwd.hpp"
#include "grid.hpp"
#include "wilderness_map.hpp"

/**
 * All structures for the game itself.
 */
struct Game {

	/*
	 * Wilderness map
	 */
	grid<wilderness_map> wilderness;

};

/**
 * Game instance
 */
extern Game *game;

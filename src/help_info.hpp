#pragma once

#include <array>

#include "h-basic.hpp"

/**
 * Maximum number of help items.
 */
constexpr int HELP_MAX = 64;

/**
 * Context help runtime data.
 */
struct help_info
{
	bool enabled = false;                     /* ingame help enabled */
	std::array<bool, HELP_MAX> activated { }; /* help item #i activated? */
};

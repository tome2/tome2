#pragma once

#include "h-basic.h"

/**
 * Maximum number of help items.
 */
constexpr int HELP_MAX = 64;

/**
 * Context help runtime data.
 */
struct help_info
{
	bool enabled = false;                  /* ingame help enabled */
	bool activated[HELP_MAX] = { false };  /* help item #i activated? */
};

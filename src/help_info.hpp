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
	bool_ enabled;              /* ingame help enabled */
	bool_ activated[HELP_MAX];  /* help item #i activated? */
};

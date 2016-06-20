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
	bool_ enabled = FALSE;                  /* ingame help enabled */
	bool_ activated[HELP_MAX] = { FALSE };  /* help item #i activated? */
};

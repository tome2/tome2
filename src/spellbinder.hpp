#pragma once

#include "h-basic.h"
#include <vector>

/**
 * Spellbinder state
 */
struct spellbinder {

	/**
	 * Bound spells.
	 */
	std::vector<u32b> spell_idxs;

	/**
	 * Trigger condition.
	 */
	byte trigger = 0;

};

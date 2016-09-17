#pragma once

#include "h-basic.h"

/**
 * Spellbinder state
 */
struct spellbinder {

	/**
	 * Number of bound spells.
	 */
	byte num = 0;

	/**
	 * Bound spells.
	 */
	u32b spells[4] = { 0 };

	/**
	 * Trigger condition.
	 */
	byte trigger = 0;

};

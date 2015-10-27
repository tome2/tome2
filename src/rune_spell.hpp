#pragma once

#include "h-basic.h"

/**
 * Runecrafter prefered spells
 */
struct rune_spell
{
	char name[30];          /* name */

	s16b type;              /* Type of the spell(GF) */
	s16b rune2;             /* Modifiers */
	s16b mana;              /* Mana involved */
};

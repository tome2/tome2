#pragma once

#include "h-basic.h"

/**
 * Runecrafter prefered spells
 */
struct rune_spell
{
	char name[30] { '\0' };             /* name */

	s16b type = 0;                      /* Type of the spell(GF) */
	s16b rune2 = 0;                     /* Modifiers */
	s16b mana = 0;                      /* Mana involved */
};

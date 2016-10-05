#pragma once

#include "h-basic.h"

/**
 * A structure to describe the random spells of the Power Mages
 */
struct random_spell
{
	char desc[30] = { };           /* Desc of the spell */
	char name[30] = { };           /* Name of the spell */
	s16b mana = 0;                 /* Mana cost */
	s16b fail = 0;                 /* Failure rate */
	u32b proj_flags = 0;           /* Project function flags */
	byte GF = 0;                   /* Type of the projection */
	byte radius = 0;
	byte dam_sides = 0;
	byte dam_dice = 0;
	byte level = 0;                /* Level needed */
	bool untried = true;           /* Is the spell was tried? */
};

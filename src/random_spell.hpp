#pragma once

#include "h-basic.h"

/**
 * A structure to describe the random spells of the Power Mages
 */
struct random_spell
{
	char desc[30];          /* Desc of the spell */
	char name[30];          /* Name of the spell */
	s16b mana;              /* Mana cost */
	s16b fail;              /* Failure rate */
	u32b proj_flags;        /* Project function flags */
	byte GF;                /* Type of the projection */
	byte radius;
	byte dam_sides;
	byte dam_dice;
	byte level;             /* Level needed */
	bool untried;           /* Is the spell was tried? */
};

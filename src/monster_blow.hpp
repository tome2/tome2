#pragma once

#include "h-basic.h"

/**
 * Monster blow descriptor.
 *
 *	- Method (RBM_*)
 *	- Effect (RBE_*)
 *	- Damage Dice
 *	- Damage Sides
 */
struct monster_blow
{
	byte method = 0;
	byte effect = 0;
	byte d_dice = 0;
	byte d_side = 0;
};

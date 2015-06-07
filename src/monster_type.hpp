#pragma once

#include "h-basic.h"
#include "monster_blow.hpp"

/**
 * Monster information for a specific monster.
 *
 * Note: fy, fx constrain dungeon size to 256x256
 *
 * The "hold_o_idx" field points to the first object of a stack
 * of objects (if any) being carried by the monster (see above).
 */
struct monster_type
{
	s16b r_idx;			/* Monster race index */

	u16b ego;                       /* Ego monster type */

	byte fy;			/* Y location on map */
	byte fx;			/* X location on map */

	s32b hp;			/* Current Hit points */
	s32b maxhp;			/* Max Hit points */

	monster_blow blow[4];           /* Up to four blows per round */

	byte speed;                     /* Speed (normally 110) */
	byte level;                     /* Level of creature */
	s16b ac;                        /* Armour Class */
	s32b exp;                       /* Experience */

	s16b csleep;		/* Inactive counter */

	byte mspeed;		/* Monster "speed" */
	byte energy;		/* Monster "energy" */

	byte stunned;		/* Monster is stunned */
	byte confused;		/* Monster is confused */
	byte monfear;		/* Monster is afraid */

	s16b bleeding;          /* Monster is bleeding */
	s16b poisoned;          /* Monster is poisoned */

	byte cdis;			/* Current dis from player */

	s32b mflag;			/* Extra monster flags */

	bool_ ml;			/* Monster is "visible" */

	s16b hold_o_idx;	/* Object being held (if any) */

	u32b smart;			/* Field for "smart_learn" */

	s16b status;                    /* Status(friendly, pet, companion, ..) */

	s16b target;                    /* Monster target */

	s16b possessor;                 /* Is it under the control of a possessor ? */
};

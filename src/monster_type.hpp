#pragma once

#include "h-basic.hpp"
#include "monster_blow.hpp"
#include "monster_race_fwd.hpp"

#include <array>
#include <cassert>
#include <vector>
#include <memory>

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
	s16b r_idx = 0;                     /* Monster race index */

	u16b ego = 0;                       /* Ego monster type */

	byte fy = 0;                        /* Y location on map */
	byte fx = 0;                        /* X location on map */

	s32b hp = 0;                        /* Current Hit points */
	s32b maxhp = 0;                     /* Max Hit points */

	std::array<monster_blow, 4> blow {};/* Up to four blows per round */

	byte speed = 0;                     /* Speed (normally 110) */
	byte level = 0;                     /* Level of creature */
	s16b ac = 0;                        /* Armour Class */
	s32b exp = 0;                       /* Experience */

	s16b csleep = 0;                    /* Inactive counter */

	byte mspeed = 0;                    /* Monster "speed" */
	byte energy = 0;                    /* Monster "energy" */

	byte stunned = 0;                   /* Monster is stunned */
	byte confused = 0;                  /* Monster is confused */
	byte monfear = 0;                   /* Monster is afraid */

	s16b bleeding = 0;                  /* Monster is bleeding */
	s16b poisoned = 0;                  /* Monster is poisoned */

	byte cdis = 0;                      /* Current dis from player */

	s32b mflag = 0;                     /* Extra monster flags */

	bool ml = false;                   /* Monster is "visible" */

	std::vector<s16b> hold_o_idxs { };  /* Objects being held */

	u32b smart = 0;                     /* Field for "smart_learn" */

	s16b status = 0;                    /* Status(friendly, pet, companion, ..) */

	s16b target = 0;                    /* Monster target */

	s16b possessor = 0;                 /* Is it under the control of a possessor ? */

	/**
	 * @brief get the "effective race" of the monster. This incorporates
	 * the effects of the "ego" of the monster, if any.
	 */
	std::shared_ptr<monster_race> race() const;

	/**
	 * @brief wipe the object's state
	 */
	void wipe()
	{
		/* Reset to defaults */
		*this = monster_type();
	}

	/**
	 * Get the o_idx of the object being mimicked
	 */
	s16b mimic_o_idx() const
	{
		// We *should* also assert that the monster has flag RF9_MIMIC,
		// but it's currently not safe since the functions we need for
		// that are a) expensive, and b) side-effecting via statics.
		assert(hold_o_idxs.size() == 1); // Mimics are defined by exactly one object
		return hold_o_idxs.front();
	}

};

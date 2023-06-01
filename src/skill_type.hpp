#pragma once

#include "h-basic.hpp"
#include "skill_flag_set.hpp"
#include "skills_defs.hpp"
#include "skill_descriptor.hpp"

/**
 * Skill runtime data.
 */
struct skill_type
{
	/**
	 * Current value.
	 */
	s32b value = 0;

	/**
	 * Current modifier, i.e. how much value 1 skill point gives.
	 */
	s32b mod = 0;

	/**
	 * Is the branch developed?
	 */
	bool dev = false;

	/**
	 * Is the skill hidden?
	 */
	bool hidden = false;
};

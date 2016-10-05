#pragma once

#include "h-basic.h"
#include "skills_defs.hpp"
#include "skill_modifier.hpp"

#include <vector>

struct skill_modifiers
{
	/**
	 * Skill modifiers indexed by skill. Note that this vector
	 * may be shorter than the s_descriptors vector.
	 */
	std::vector<skill_modifier> modifiers;

};

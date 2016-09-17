#pragma once

#include "h-basic.h"
#include "skills_defs.hpp"
#include "skill_modifier.hpp"
#include <array>

struct skill_modifiers
{
	std::array<skill_modifier, MAX_SKILLS> modifiers;
};

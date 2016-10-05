#pragma once

#include <array>
#include <string>

#include "set_component.hpp"

/**
 * Item set descriptor and runtime information.
 */
struct set_type
{
	std::string name;                        /* Name */
	std::string desc;                        /* Desc */

	byte num = 0;                            /* Number of artifacts used */
	byte num_use = 0;                        /* Number actually worn */

	std::array<set_component, SET_MAX_SIZE> arts;
};

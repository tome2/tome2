#pragma once

#include <array>
#include "set_component.hpp"

/**
 * Item set descriptor and runtime information.
 */
struct set_type
{
	const char *name = nullptr;              /* Name */
	char *desc = nullptr;                    /* Desc */

	byte num = 0;                            /* Number of artifacts used */
	byte num_use = 0;                        /* Number actually worn */

	std::array<set_component, SET_MAX_SIZE> arts;
};

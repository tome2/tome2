#pragma once

#include "h-basic.h"
#include "object_flag_set.hpp"

#include <array>
#include <cstdint>

constexpr std::size_t SET_MAX_SIZE = 6;

struct set_component {
	bool_ present = FALSE;                             /* Is it being worn? */
	s16b a_idx = 0;                                    /* What artifact? */
	std::array<s16b           , SET_MAX_SIZE> pval;    /* Pval for each combination */
	std::array<object_flag_set, SET_MAX_SIZE> flags;   /* Flags */
};

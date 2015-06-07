#pragma once

#include "range_fwd.hpp"
#include "h-basic.h"

/*
 * Range
 */
struct range_type
{
	s32b min;
	s32b max;
};

void range_init(range_type *range, s32b min, s32b max);

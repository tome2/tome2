#pragma once

#include "range_fwd.h"
#include "angband.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Range
 */
struct range_type
{
	s32b min;
	s32b max;
};

void range_init(range_type *range, s32b min, s32b max);

#ifdef __cplusplus
} // extern "C"
#endif

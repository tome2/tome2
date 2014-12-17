#pragma once

#include "device_allocation_fwd.h"
#include "range.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Device allocation for skill
 */
struct device_allocation
{
	byte tval;
	s32b rarity;
	range_type base_level;
	range_type max_level;
};

struct device_allocation *device_allocation_new(byte tval);

#ifdef __cplusplus
} // extern "C"
#endif

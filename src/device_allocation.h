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
	/* Next device allocation in the list */
	device_allocation *next;
};

int compare_device_allocation(device_allocation *a, device_allocation *b);
SGLIB_DEFINE_LIST_PROTOTYPES(device_allocation, compare_device_allocation, next);

void device_allocation_init(struct device_allocation *device_allocation, byte tval);
struct device_allocation *device_allocation_new(byte tval);

#ifdef __cplusplus
} // extern "C"
#endif

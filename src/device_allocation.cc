#include "device_allocation.h"

#include <cassert>

static void device_allocation_init(device_allocation *device_allocation, byte tval)
{
	assert(device_allocation != NULL);

	device_allocation->tval = tval;
	device_allocation->rarity = 0;
	range_init(&device_allocation->base_level, 0, 0);
	range_init(&device_allocation->max_level, 0, 0);
}

device_allocation *device_allocation_new(byte tval)
{
	device_allocation *d = new device_allocation;
	device_allocation_init(d, tval);
	return d;
}

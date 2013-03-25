#include "device_allocation.h"

int compare_device_allocation(device_allocation *a, device_allocation *b)
{
	return SGLIB_NUMERIC_COMPARATOR(a->tval, b->tval);
}

SGLIB_DEFINE_LIST_FUNCTIONS(device_allocation, compare_device_allocation, next);

void device_allocation_init(device_allocation *device_allocation, byte tval)
{
	assert(device_allocation != NULL);

	device_allocation->tval = tval;
	device_allocation->rarity = 0;
	range_init(&device_allocation->base_level, 0, 0);
	range_init(&device_allocation->max_level, 0, 0);
	device_allocation->next = NULL;
}

device_allocation *device_allocation_new(byte tval)
{
	device_allocation *d = malloc(sizeof(device_allocation));
	device_allocation_init(d, tval);
	return d;
}

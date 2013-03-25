#pragma once

#include "angband.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct device_allocation device_allocation;
struct device_allocation;

void device_allocation_init(struct device_allocation *device_allocation, byte tval);
struct device_allocation *device_allocation_new(byte tval);

#ifdef __cplusplus
} // extern "C"
#endif

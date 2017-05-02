#pragma once

#include "h-basic.h"

// C linkage required for these functions since main-* code uses them.
#ifdef __cplusplus
extern "C" {
#endif

void save_dungeon();
bool_ save_player();

#ifdef __cplusplus
} // extern "C"
#endif

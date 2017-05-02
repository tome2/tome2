#pragma once

#include "h-basic.h"

// C linkage required for these functions since main-* code uses them.
#ifdef __cplusplus
extern "C" {
#endif

void save_dungeon(void);
bool_ save_player(void);

#ifdef __cplusplus
} // extern "C"
#endif

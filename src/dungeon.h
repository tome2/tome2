#pragma once

#include "h-basic.h"

// C linkage required for these functions since main-* code uses them.
#ifdef __cplusplus
extern "C" {
#endif

extern void play_game();

#ifdef __cplusplus
} // extern "C"
#endif

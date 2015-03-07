#pragma once

#include "h-basic.h"

// C linkage required for these functions since main-* code uses them.
#ifdef __cplusplus
extern "C" {
#endif

/* loadsave.c */
extern void save_dungeon(void);
extern bool_ save_player(void);

#ifdef __cplusplus
} // extern "C"
#endif

#pragma once

// C linkage required for these functions since main-* code uses them.
#ifdef __cplusplus
extern "C" {
#endif

extern void init_lua_init(void);

#ifdef __cplusplus
} // extern "C"
#endif

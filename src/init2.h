#pragma once

// C linkage required for these functions since main-* code uses them.
#ifdef __cplusplus
extern "C" {
#endif

extern void init_file_paths(char *path);
extern void init_file_paths_with_env();
extern void init_angband(void);

#ifdef __cplusplus
} // extern "C"
#endif

#pragma once

// C linkage required for these functions since main-* code uses them.
#ifdef __cplusplus
extern "C" {
#endif

void init_file_paths(char *path);
void init_angband(void);

#ifdef __cplusplus
} // extern "C"
#endif

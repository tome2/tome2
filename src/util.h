#pragma once

#include "h-basic.h"

// C linkage required for these functions since main-* code uses them.
#ifdef __cplusplus
extern "C" {
#endif

errr path_build(char *buf, int max, const char *path, const char *file);
void bell();
errr macro_add(const char *pat, const char *act);
int macro_find_exact(const char *pat);
char inkey();
void prt(const char *str, int row, int col);

#ifdef __cplusplus
} // extern "C"
#endif

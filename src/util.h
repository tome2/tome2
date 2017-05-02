#pragma once

#include "h-basic.h"

// C linkage required for these functions since main-* code uses them.
#ifdef __cplusplus
extern "C" {
#endif

errr path_build(char *buf, int max, cptr path, cptr file);
void bell(void);
errr macro_add(cptr pat, cptr act);
sint macro_find_exact(cptr pat);
char inkey(void);
void prt(cptr str, int row, int col);

#ifdef __cplusplus
} // extern "C"
#endif

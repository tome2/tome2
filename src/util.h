#pragma once

#include "h-basic.h"

// C linkage required for these functions since main-* code uses them.
#ifdef __cplusplus
extern "C" {
#endif

extern errr path_parse(char *buf, int max, cptr file);
extern errr path_build(char *buf, int max, cptr path, cptr file);
extern void bell(void);
extern errr macro_add(cptr pat, cptr act);
extern sint macro_find_exact(cptr pat);
extern char inkey(void);
extern void prt(cptr str, int row, int col);
extern void pause_line(int row);

#ifdef __cplusplus
} // extern "C"
#endif

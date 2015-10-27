#pragma once

#include "h-basic.h"

// C linkage required for these functions since main-* code uses them.
#ifdef __cplusplus
extern "C" {
#endif

extern bool_ private_check_user_directory(cptr dirpath);
extern cptr force_module;

#ifdef __cplusplus
} // extern "C"
#endif

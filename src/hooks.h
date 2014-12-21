#pragma once

#include "angband.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef bool_ (*hook_func_t)(void *, void *, void *);

extern void add_hook_new(int h_idx, hook_func_t hook_func, cptr name, void *data);
extern void del_hook_new(int h_idx, hook_func_t hook_func);
extern int process_hooks_restart;
extern bool_ process_hooks_new(int h_idx, void *in, void *out);

#ifdef __cplusplus
} // extern "C"
#endif

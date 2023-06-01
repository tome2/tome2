#pragma once

#include "h-basic.hpp"

typedef bool (*hook_func_t)(void *, void *, void *);

void add_hook_new(int h_idx, hook_func_t hook_func, const char *name, void *data);
void del_hook_new(int h_idx, hook_func_t hook_func);
extern int process_hooks_restart;
bool process_hooks_new(int h_idx, void *in, void *out);

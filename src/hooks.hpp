#pragma once

#include "h-basic.h"

typedef bool_ (*hook_func_t)(void *, void *, void *);

void add_hook_new(int h_idx, hook_func_t hook_func, cptr name, void *data);
void del_hook_new(int h_idx, hook_func_t hook_func);
extern int process_hooks_restart;
bool_ process_hooks_new(int h_idx, void *in, void *out);

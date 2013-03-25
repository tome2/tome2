#pragma once

#include "angband.h"

extern FILE *hook_file;
extern void wipe_hooks(void);
extern void init_hooks(void);
extern hooks_chain* add_hook(int h_idx, hook_type hook, cptr name);
extern void add_hook_new(int h_idx, bool_ (*hook_f)(void *, void *, void *), cptr name, void *data);
extern void del_hook(int h_idx, hook_type hook);
extern s32b get_next_arg(const char *fmt);
extern char* get_next_arg_str(const char *fmt);
extern object_type *get_next_arg_obj();
extern int process_hooks_restart;
extern hook_return process_hooks_return[20];
extern bool_ process_hooks_ret(int h_idx, char *ret, char *fmt, ...);
extern bool_ process_hooks(int h_idx, char *fmt, ...);
extern bool_ process_hooks_new(int h_idx, void *in, void *out);

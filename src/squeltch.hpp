#pragma once

#include "angband.h"

extern void squeltch_inventory(void);
extern void squeltch_grid(void);
extern void do_cmd_automatizer(void);
extern void automatizer_add_rule(object_type *o_ptr);
extern bool_ automatizer_create;
extern void automatizer_init();
extern void automatizer_load(cptr file_name);


#pragma once

#include "h-basic.h"
#include "object_type_fwd.hpp"
#include <boost/filesystem.hpp>

extern void squeltch_inventory(void);
extern void squeltch_grid(void);
extern void do_cmd_automatizer(void);
extern void automatizer_add_rule(object_type *o_ptr);
extern bool_ automatizer_create;
extern void automatizer_init();
extern bool automatizer_load(boost::filesystem::path const &path);

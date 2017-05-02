#pragma once

#include "h-basic.h"
#include "object_type_fwd.hpp"
#include <boost/filesystem.hpp>

void squeltch_inventory(void);
void squeltch_grid(void);
void do_cmd_automatizer(void);
void automatizer_add_rule(object_type *o_ptr);
extern bool_ automatizer_create;
void automatizer_init();
bool automatizer_load(boost::filesystem::path const &path);

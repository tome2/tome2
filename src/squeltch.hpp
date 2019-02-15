#pragma once

#include "object_type_fwd.hpp"
#include <boost/filesystem.hpp>

void squeltch_inventory();
void squeltch_grid();
void do_cmd_automatizer();
void automatizer_add_rule(object_type *o_ptr);
extern bool automatizer_create;
void automatizer_init();
bool automatizer_load(boost::filesystem::path const &path);

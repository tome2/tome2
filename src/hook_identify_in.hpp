#pragma once

#include "object_type_fwd.hpp"
#include "identify_mode.hpp"

struct hook_identify_in {
	object_type *o_ptr;
	identify_mode mode;
};

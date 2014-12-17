#pragma once

#include "spell_idx_list_fwd.h"
#include <vector>
// FIXME: h-basic for the s32b?

struct spell_idx_list {
	// FIXME: stupidity because we can't "override" a fwd-declared-struct directly
	std::vector<s32b> v;
};


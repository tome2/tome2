#pragma once

#include "h-basic.hpp"

struct random_quest
{
	byte type;              /* Type/number of monsters to kill(0 = no quest) */
	s16b r_idx;             /* Monsters to crush */
	bool done;              /* Done? */
};

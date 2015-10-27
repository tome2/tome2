#pragma once

#include "h-basic.h"

/*
 * Timer descriptor and runtime data.
 */
struct timer_type
{
	timer_type *next;       /* The next timer in the list */

	bool_ enabled;           /* Is it currently counting? */

	s32b delay;             /* Delay between activations */
	s32b countdown;         /* The current number of turns passed, when it reaches delay it fires */

	void (*callback)();   /* The C function to call upon firing */
};

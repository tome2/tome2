#pragma once

#include "h-basic.h"

/**
 * Store/building actions.
 */
struct store_action_type
{
	const char *name;               /* Name */

	s16b costs[3];                  /* Costs for liked people */
	char letter;                    /* Action letter */
	char letter_aux;                /* Action letter */
	s16b action;                    /* Action code */
	s16b action_restr;              /* Action restriction */
};

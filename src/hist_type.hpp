#pragma once

#include "h-basic.h"

/**
 * Player background descriptor.
 */
struct hist_type
{
	char *info;                             /* Textual History */

	byte roll;			        /* Frequency of this entry */
	s16b chart;                             /* Chart index */
	s16b next;                              /* Next chart index */
	byte bonus;			        /* Social Class Bonus + 50 */
};

#pragma once

#include "h-basic.h"

/**
 * Martial arts descriptors
 */
struct martial_arts
{
	cptr    desc;    /* A verbose attack description */
	int     min_level;  /* Minimum level to use */
	int     chance;     /* Chance of 'success' */
	int     dd;        /* Damage dice */
	int     ds;        /* Damage sides */
	s16b    effect;     /* Special effects */
	s16b    power;     /* Special effects power */
};


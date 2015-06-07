#pragma once

#include "h-basic.h"

/**
 * Random artifact descriptor.
 */
struct random_artifact
{
	char name_full[80];     /* Full name for the artifact */
	char name_short[80];    /* Un-Id'd name */
	byte level;             /* Level of the artifact */
	byte attr;              /* Color that is used on the screen */
	u32b cost;              /* Object's value */
	byte activation;        /* Activation. */
	s16b timeout;           /* Timeout. */
	byte generated;         /* Does it exist already? */
};

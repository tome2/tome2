#pragma once

#include "h-basic.hpp"

#include <string>

/**
 * Random artifact descriptor.
 */
struct random_artifact
{
	std::string name_full;  /* Full name for the artifact */
	std::string name_short; /* Un-Id'd name */
	byte level;             /* Level of the artifact */
	byte attr;              /* Color that is used on the screen */
	u32b cost;              /* Object's value */
	byte activation;        /* Activation. */
	s16b timeout;           /* Timeout. */
	byte generated;         /* Does it exist already? */
};

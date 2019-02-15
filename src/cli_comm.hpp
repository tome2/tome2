#pragma once

#include "h-basic.hpp"

/**
 * A structure for CLI commands.
 */
struct cli_comm
{
	const char *comm;	/* Extended name of the command. */
	const char *descrip;	/* Description of the command. */
	s16b key;	/* Key to convert command to. */
};

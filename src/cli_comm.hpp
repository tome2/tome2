#pragma once

#include "h-basic.h"

/**
 * A structure for CLI commands.
 */
struct cli_comm
{
	cptr comm;	/* Extended name of the command. */
	cptr descrip;	/* Description of the command. */
	s16b key;	/* Key to convert command to. */
};

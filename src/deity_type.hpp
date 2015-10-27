#pragma once

/**
 * A structure for deity information.
 */
struct deity_type
{
	int  modules[3]; /* terminated with -1 */
	char const *name;
	char desc[10][80];
};

#pragma once

#include "h-basic.h"

/**
 * Vault descriptors.
 */
struct vault_type
{
	char *data;			/* Vault data */

	byte typ;			/* Vault type */

	byte rat;			/* Vault rating */

	byte hgt;			/* Vault height */
	byte wid;			/* Vault width */

	s16b lvl;                       /* level of special (if any) */
	byte dun_type;                  /* Dungeon type where the level will show up */

	s16b mon[10];                   /* special monster */
	int item[3];                   /* number of item (usually artifact) */
};

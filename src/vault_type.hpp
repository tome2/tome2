#pragma once

#include "h-basic.h"
#include <string>

/**
 * Vault descriptors.
 */
struct vault_type
{
	std::string data;		/* Vault data */

	byte typ = 0;			/* Vault type */

	byte rat = 0;			/* Vault rating */

	byte hgt = 0;			/* Vault height */
	byte wid = 0;			/* Vault width */

	s16b lvl = 0;                   /* level of special (if any) */
	byte dun_type = 0;              /* Dungeon type where the level will show up */

	s16b mon[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };                   /* special monster */
	int item[3] = { 0, 0, 0 };                   /* number of item (usually artifact) */
};

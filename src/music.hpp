#pragma once

#include "h-basic.hpp"

/**
 * Music descriptor.
 */
struct music
{
	char desc[80];          /* Desc of the music */
	s16b music;             /* Music. */
	s16b dur;               /* Duration(if any) */
	s16b init_recharge;     /* Minimal recharge time */
	s16b turn_recharge;     /* Recharge time for each more turn */
	byte min_inst;          /* Minimum instrument for the music */
	byte rarity;            /* Rarity of the music(use 100 to unallow to be randomly generated) */
};

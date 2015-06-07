#pragma once

#include "h-basic.h"

/**
 * Fate descritpor.
 */
struct fate
{
	byte fate;      /* Which fate */
	byte level;     /* On which level */
	byte serious;   /* Is it sure? */
	s16b o_idx;     /* Object to find */
	s16b e_idx;     /* Ego-Item to find */
	s16b a_idx;     /* Artifact to find */
	s16b v_idx;     /* Vault to find */
	s16b r_idx;     /* Monster to find */
	s16b count;     /* Number of things */
	s16b time;      /* Turn before */
	bool_ know;      /* Has it been predicted? */
	bool_ icky;	/* Hackish runtime-only flag */
};

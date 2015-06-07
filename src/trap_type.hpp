#pragma once

#include "h-basic.h"

/**
 * Trap descriptor.
 */
struct trap_type
{
	s16b probability;      /* probability of existence */
	s16b another;          /* does this trap easily combine */
	s16b p1valinc;         /* how much does this trap attribute to p1val */
	byte difficulty;       /* how difficult to disarm */
	byte minlevel;         /* what is the minimum level on which the traps should be */
	byte color;            /* what is the color on screen */
	u32b flags;            /* where can these traps go - and perhaps other flags */
	bool_ ident;           /* do we know the name */
	s16b known;            /* how well is this trap known */
	const char *name;      /* normal name like weakness */
	s16b dd, ds;           /* base damage */
	char *text;            /* longer description once you've met this trap */
	byte g_attr;           /* Overlay graphic attribute */
	char g_char;           /* Overlay graphic character */
};

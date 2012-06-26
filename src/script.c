/* File: script.c */

/* Purpose: scripting in lua */

/*
 * Copyright (c) 2001 Dark God
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"


void init_lua_init()
{
	/* Initialize schooled spells */
	schools_init();
	school_spells_init();
	init_school_books();

	/* Post-spell creation initialization */
	initialize_bookable_spells();

	/* Finish up the corruptions */
	init_corruptions();
}

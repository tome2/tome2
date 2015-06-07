#pragma once

#include "h-basic.h"

/**
 * A single "grid" in a Cave.
 *
 * Note that several aspects of the code restrict the actual cave
 * to a max size of 256 by 256.  In partcular, locations are often
 * saved as bytes, limiting each coordinate to the 0-255 range.
 *
 * The "o_idx" and "m_idx" fields are very interesting.  There are
 * many places in the code where we need quick access to the actual
 * monster or object(s) in a given cave grid.  The easiest way to
 * do this is to simply keep the index of the monster and object
 * (if any) with the grid, but this takes 198*66*4 bytes of memory.
 * Several other methods come to mind, which require only half this
 * amound of memory, but they all seem rather complicated, and would
 * probably add enough code that the savings would be lost.  So for
 * these reasons, we simply store an index into the "o_list" and
 * "m_list" arrays, using "zero" when no monster/object is present.
 *
 * Note that "o_idx" is the index of the top object in a stack of
 * objects, using the "next_o_idx" field of objects to create the
 * singly linked list of objects.  If "o_idx" is zero then there
 * are no objects in the grid.
 */
struct cave_type
{
	u16b info;		/* Hack -- cave flags */

	byte feat;		/* Hack -- feature type */

	s16b o_idx;		/* Object in this grid */

	s16b m_idx;		/* Monster in this grid */

	s16b t_idx;		/* trap index (in t_list) or zero       */

	s16b special, special2; /* Special cave info */

	s16b inscription;       /* Inscription of the grid */

	byte mana;              /* Magical energy of the grid */

	byte mimic;		/* Feature to mimic */

	byte cost;		/* Hack -- cost of flowing */
	byte when;		/* Hack -- when cost was computed */

	s16b effect;            /* The lasting effects */
};

#pragma once

#include "h-basic.hpp"

#include <boost/optional.hpp>
#include <cassert>
#include <vector>

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
	u16b info = 0;	                       /* Hack -- cave flags */

	byte feat = 0;                         /* Hack -- feature type */

	std::vector<s16b> o_idxs { };          /* Indexes of objects in this grid */

	s16b m_idx = 0;                        /* Monster in this grid */

	s16b special = 0;                      /* Special cave info */
	s16b special2 = 0;                     /* Special cave info */

	s16b inscription = 0;                  /* Inscription of the grid */

	byte mana = 0;                         /* Magical energy of the grid */

	byte mimic = 0;                        /* Feature to mimic */

	byte cost = 0;                         /* Hack -- cost of flowing */
	byte when = 0;                         /* Hack -- when cost was computed */

	boost::optional<s16b> maybe_effect { }; /* The lasting effects */

	/**
	 * @brief wipe the object's state
	 */
	void wipe() {
		/* Reset to defaults */
		*this = cave_type();
	}

};

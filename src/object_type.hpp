#pragma once

#include "h-basic.h"
#include "object_flag_set.hpp"

#include <string>

/**
 * Object information for a specific object.
 *
 * Note that a "discount" on an item is permanent and never goes away.
 *
 * Note that inscriptions are now handled via the "quark_str()" function
 * applied to the "note" field, which will return NULL if "note" is zero.
 *
 * Note that "object" records are "copied" on a fairly regular basis,
 * and care must be taken when handling such objects.
 *
 * Note that "object flags" must now be derived from the object kind,
 * the artifact and ego-item indexes, and the two "xtra" fields.
 *
 * Each cave grid points to one (or zero) objects via the "o_idx"
 * field (above).  Each object then points to one (or zero) objects
 * via the "next_o_idx" field, forming a singly linked list, which
 * in game terms, represents a "stack" of objects in the same grid.
 *
 * Each monster points to one (or zero) objects via the "hold_o_idx"
 * field (below).  Each object then points to one (or zero) objects
 * via the "next_o_idx" field, forming a singly linked list, which
 * in game terms, represents a pile of objects held by the monster.
 *
 * The "held_m_idx" field is used to indicate which monster, if any,
 * is holding the object.  Objects being held have "ix=0" and "iy=0".
 */
struct object_type
{
	s16b k_idx = 0;                          /* Kind index (zero if "dead") */

	byte iy = 0;                             /* Y-position on map, or zero */
	byte ix = 0;                             /* X-position on map, or zero */

	byte tval = 0;                           /* Item type (from kind) */
	byte sval = 0;                           /* Item sub-type (from kind) */

	s32b pval = 0;                           /* Item extra-parameter */
	s16b pval2 = 0;                          /* Item extra-parameter for some special items */
	s32b pval3 = 0;                          /* Item extra-parameter for some special items */

	byte discount = 0;                       /* Discount (if any) */

	byte number = 0;                         /* Number of items */

	s32b weight = 0;                         /* Item weight */

	byte elevel = 0;                         /* Item exp level */
	s32b exp = 0;                            /* Item exp */

	byte name1 = 0;                          /* Artifact type, if any */
	s16b name2 = 0;                          /* Ego-Item type, if any */
	s16b name2b = 0;                         /* Second Ego-Item type, if any */

	byte xtra1 = 0;                          /* Extra info type */
	s16b xtra2 = 0;                          /* Extra info index */

	s16b to_h = 0;                           /* Plusses to hit */
	s16b to_d = 0;                           /* Plusses to damage */
	s16b to_a = 0;                           /* Plusses to AC */

	s16b ac = 0;                             /* Normal AC */

	byte dd = 0;                             /* Damage dice/sides */
	byte ds = 0;                             /* Damage dice/sides */

	s16b timeout = 0;                        /* Timeout Counter */

	byte ident = 0;                          /* Special flags  */

	byte marked = 0;                         /* Object is marked */

	std::string inscription;                 /* Inscription index */

	u16b art_name = 0;                       /* Artifact name (random artifacts) */

	object_flag_set art_flags;               /* Flags */
	object_flag_set art_oflags;              /* Obvious flags */

	s16b held_m_idx = 0;                     /* Monster holding the object; if any */

	byte sense = 0;                          /* Pseudo-id status */

	byte found = 0;                          /* How did we find it */
	s16b found_aux1 = 0;                     /* Stores info for found */
	s16b found_aux2 = 0;                     /* Stores info for found */
	s16b found_aux3 = 0;                     /* Stores info for found */
	s16b found_aux4 = 0;                     /* Stores info for found */
};

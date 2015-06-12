#pragma once

#include "h-basic.h"

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
	s16b k_idx;			/* Kind index (zero if "dead") */

	byte iy;			/* Y-position on map, or zero */
	byte ix;			/* X-position on map, or zero */

	byte tval;			/* Item type (from kind) */
	byte sval;			/* Item sub-type (from kind) */

	s32b pval;                      /* Item extra-parameter */
	s16b pval2;                     /* Item extra-parameter for some special
					   items*/
	s32b pval3;                     /* Item extra-parameter for some special
					   items*/

	byte discount;		/* Discount (if any) */

	byte number;		/* Number of items */

	s32b weight;            /* Item weight */

	byte elevel;            /* Item exp level */
	s32b exp;               /* Item exp */

	byte name1;			/* Artifact type, if any */
	s16b name2;                     /* Ego-Item type, if any */
	s16b name2b;                    /* Second Ego-Item type, if any */

	byte xtra1;			/* Extra info type */
	s16b xtra2;                     /* Extra info index */

	s16b to_h;			/* Plusses to hit */
	s16b to_d;			/* Plusses to damage */
	s16b to_a;			/* Plusses to AC */

	s16b ac;			/* Normal AC */

	byte dd, ds;		/* Damage dice/sides */

	s16b timeout;		/* Timeout Counter */

	byte ident;			/* Special flags  */

	byte marked;		/* Object is marked */

	u16b note;			/* Inscription index */
	u16b art_name;      /* Artifact name (random artifacts) */

	u32b art_flags1;        /* Flags, set 1  Alas, these were necessary */
	u32b art_flags2;        /* Flags, set 2  for the random artifacts of*/
	u32b art_flags3;        /* Flags, set 3  Zangband */
	u32b art_flags4;        /* Flags, set 4  PernAngband */
	u32b art_flags5;        /* Flags, set 5  PernAngband */
	u32b art_esp;           /* Flags, set esp  PernAngband */

	u32b art_oflags1;       /* Obvious Flags, set 1 */
	u32b art_oflags2;       /* Obvious Flags, set 2 */
	u32b art_oflags3;       /* Obvious Flags, set 3 */
	u32b art_oflags4;       /* Obvious Flags, set 4 */
	u32b art_oflags5;       /* Obvious Flags, set 5 */
	u32b art_oesp;          /* Obvious Flags, set esp */

	s16b held_m_idx;	/* Monster holding us (if any) */

	byte sense;             /* Pseudo-id status */

	byte found;             /* How did we find it */
	s16b found_aux1;        /* Stores info for found */
	s16b found_aux2;        /* Stores info for found */
	s16b found_aux3;        /* Stores info for found */
	s16b found_aux4;        /* Stores info for found */
};

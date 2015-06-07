#pragma once

#include "h-basic.h"

/**
 * Item set descriptor and runtime information.
 */
struct set_type
{
	const char *name;                       /* Name */
	char *desc;                             /* Desc */

	byte num;                               /* Number of artifacts used */
	byte num_use;                           /* Number actually wore */

	struct                                  /* the various items */
	{
		bool_ present;                   /* Is it actually wore ? */
		s16b a_idx;                     /* What artifact ? */
		s16b pval[6];                   /* Pval for each combination */
		u32b flags1[6];                 /* Flags */
		u32b flags2[6];                 /* Flags */
		u32b flags3[6];                 /* Flags */
		u32b flags4[6];                 /* Flags */
		u32b flags5[6];                 /* Flags */
		u32b esp[6];                    /* Flags */
	} arts[6];
};

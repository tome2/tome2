#pragma once

#include "h-basic.h"

/**
 * For level gaining artifacts
 */
struct flags_group
{
	char name[30];          /* Name */
	byte color;             /* Color */

	byte price;             /* Price to "buy" it */

	u32b flags1;            /* Flags set 1 */
	u32b flags2;            /* Flags set 2 */
	u32b flags3;            /* Flags set 3 */
	u32b flags4;            /* Flags set 4 */
	u32b esp;               /* ESP flags set */
};

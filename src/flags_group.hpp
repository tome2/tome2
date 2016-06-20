#pragma once

#include "h-basic.h"

/**
 * For level gaining artifacts
 */
struct flags_group
{
	char name[30] { };                       /* Name */
	byte color = 0;                          /* Color */

	byte price = 0;                          /* Price to "buy" it */

	u32b flags1 = 0;                         /* Flags set 1 */
	u32b flags2 = 0;                         /* Flags set 2 */
	u32b flags3 = 0;                         /* Flags set 3 */
	u32b flags4 = 0;                         /* Flags set 4 */
	u32b esp = 0;                            /* ESP flags set */
};

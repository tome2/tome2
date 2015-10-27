#pragma once

#include "h-basic.h"

/**
 * Inscriptions
 */
struct inscription_info_type
{
	char text[40];                  /* The inscription itself */
	byte when;                      /* When it is executed */
	bool_ know;                      /* Is the inscription know ? */
	byte mana;                      /* Grid mana needed */
};

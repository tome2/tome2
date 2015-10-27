#pragma once

#include "h-basic.h"

/**
 * Object theme. Probability in percent for each class of
 * objects to be dropped.
 */
struct obj_theme
{
	byte treasure;
	byte combat;
	byte magic;
	byte tools;
};

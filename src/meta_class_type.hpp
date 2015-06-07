#pragma once

#include "h-basic.h"

struct meta_class_type
{
	char name[80];     /* Name */
	byte color;
	s16b *classes;          /* list of classes */
};

#pragma once

#include "h-basic.h"

struct meta_class_type
{
	char name[80] = "";                      /* Name */
	byte color = 0;
	s16b *classes = nullptr;                 /* List of classes */
};

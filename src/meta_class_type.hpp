#pragma once

#include "h-basic.h"
#include <vector>

struct meta_class_type
{
	char name[80] = "";                      /* Name */
	byte color = 0;
	std::vector<u16b> classes;               /* List of classes */
};

#pragma once

#include "h-basic.h"

/**
 * Object theme. Probability in percent for each class of
 * objects to be dropped.
 */
struct obj_theme
{
	byte treasure = 0;
	byte combat = 0;
	byte magic = 0;
	byte tools = 0;

	bool operator == (obj_theme const &other) const
	{
		return
			(treasure == other.treasure) &&
			(combat == other.combat) &&
			(magic == other.magic) &&
			(tools == other.tools);
	}

	bool operator != (obj_theme const &other) const
	{
		return !(*this == other);
	}

	static constexpr obj_theme no_theme()
	{
		return equal_spread(100);
	}

	static constexpr obj_theme defaults()
	{
		return equal_spread(20);
	}

private:

	static constexpr obj_theme equal_spread(byte v)
	{
		obj_theme ot;
		ot.treasure = v;
		ot.combat = v;
		ot.magic = v;
		ot.tools = v;
		return ot;
	}

};

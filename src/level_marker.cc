#include "level_marker.hpp"

EnumStringMap<level_marker> const &level_marker_values()
{
	auto static instance = new EnumStringMap<level_marker> {
		{ level_marker::NORMAL, "normal" },
		{ level_marker::SPECIAL, "special" },
		{ level_marker::REGENERATE, "regenerate" }
	};

	return *instance;
}

#pragma once

#include <tome/enum_string_map.hpp>

/**
 * Markers for 'special' levels.
 */
enum class level_marker {
	NORMAL,
	SPECIAL,
	REGENERATE
};

/**
 * Is the level "normal"?
 */
inline bool is_normal_level(level_marker m)
{
	return m == level_marker::NORMAL;
}

/**
 * Bidrectional map between enum and strings.
 */
EnumStringMap<level_marker> const &level_marker_values();

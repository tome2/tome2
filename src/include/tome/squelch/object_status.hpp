#ifndef H_e3f9ebbe_ff9a_4687_a847_6101f094b483
#define H_e3f9ebbe_ff9a_4687_a847_6101f094b483

#include "tome/enum_string_map.hpp"

namespace squelch {

/**
 * Types of statuses for objects, e.g. "special" for artifacts and
 * "average" for plain objects with no plusses.
 */
enum class status_type {
	BAD    , VERY_BAD, AVERAGE, GOOD,        VERY_GOOD,
	SPECIAL, TERRIBLE, NONE,    CHEST_EMPTY, CHEST_DISARMED };

/**
 * Bidirectional map between status_type values and strings.
 */
EnumStringMap<status_type> &status_mapping();

/**
 * Find the status of an object
 */
status_type object_status(object_type *o_ptr);

} // namespace

#endif

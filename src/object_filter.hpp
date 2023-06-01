#pragma once

#include "h-basic.hpp"
#include "object_flag_set.hpp"
#include "object_type_fwd.hpp"

#include <functional>
#include <initializer_list>

typedef std::function<bool (object_type const *)> object_filter_t;

namespace object_filter {

/**
 * Is TVal equal to the given value?
 */
object_filter_t TVal(byte tval);

/**
 * Is SVal equal to the given value?
 */
object_filter_t SVal(byte sval);

/**
 * Has given set of flags set.
 */
object_filter_t HasFlags(object_flag_set const &);

/**
 * Is the object an artifact?
 */
object_filter_t IsArtifact();

/**
 * Is the object an artifact as determined by artifact_p?
 */
object_filter_t IsArtifactP();

/**
 * Is the object an ego item?
 */
object_filter_t IsEgo();

/**
 * Is the object "known"?
 */
object_filter_t IsKnown();

/**
 * True always accepts all items.
 */
object_filter_t True();

/**
 * Invert an object filter.
 */
object_filter_t Not(object_filter_t p);

/**
 * Logical conjunction (AND)
 */
object_filter_t And();

/**
 * Logical conjunction (AND)
 */
template<typename Arg0, typename... Args> object_filter_t And(Arg0&& arg0, Args&&... args) {
	auto argsFilter = And(args...);
	return [=](object_type const *o_ptr) -> bool {
		return arg0(o_ptr) && argsFilter(o_ptr);
	};
}

/**
 * Logical disjunction (OR)
 */
object_filter_t Or();

/**
 * Logical disjunction (OR)
 */
template<typename Arg0, typename... Args> object_filter_t Or(Arg0&& arg0, Args&&... args) {
	auto argsFilter = Or(args...);
	return [=](object_type const *o_ptr) -> bool {
		auto x = arg0(o_ptr) || argsFilter(o_ptr);
		return x;
	};
}

}

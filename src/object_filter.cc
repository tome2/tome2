#include "object_filter.hpp"
#include "object1.hpp"
#include "object2.hpp"
#include "object_type.hpp"

namespace object_filter {

object_filter_t TVal(byte tval) {
	return [=](object_type const *o_ptr) -> bool {
		return o_ptr->tval == tval;
	};
}

object_filter_t SVal(byte sval) {
	return [=](object_type const *o_ptr) -> bool {
		return o_ptr->sval == sval;
	};
}

object_filter_t HasFlags(object_flag_set const &mask) {
	return [=](object_type const *o_ptr) -> bool {
		auto const flags = object_flags(o_ptr);
		return bool(flags & mask);
	};
}

object_filter_t IsArtifact() {
	return [](object_type const *o_ptr) -> bool {
		return o_ptr->name1 > 0;
	};
}

object_filter_t IsArtifactP() {
	return [](object_type const *o_ptr) -> bool {
		return artifact_p(o_ptr);
	};
}

object_filter_t IsEgo() {
	return [](object_type const *o_ptr) -> bool {
		return ego_item_p(o_ptr);
	};
}

object_filter_t IsKnown() {
	return [](object_type const *o_ptr) -> bool {
		return object_known_p(o_ptr);
	};
}

object_filter_t True() {
	return [](object_type const *o_ptr) -> bool {
		return true;
	};
}

object_filter_t Not(object_filter_t p) {
	return [=](object_type const *o_ptr) -> bool {
		return !p(o_ptr);
	};
}

object_filter_t And() {
	return [](object_type const *) -> bool {
		return true;
	};
}

object_filter_t Or() {
	return [](object_type const *) -> bool {
		return false;
	};
}

}

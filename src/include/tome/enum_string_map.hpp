#pragma once

#include <boost/bimap.hpp>
#include <boost/noncopyable.hpp>
#include <string>
#include <cassert>

/**
 * Bidirectional mapping between enumerated values
 * and strings.
 */
template <class E>
class EnumStringMap : boost::noncopyable {

private:
	typedef boost::bimap< E, std::string > bimap_type;
	typedef typename bimap_type::value_type value_type;

	bimap_type bimap;

public:
	explicit EnumStringMap(std::initializer_list< std::pair<E, const char *> > in) {
		for (auto es : in)
		{
			bimap.insert(value_type(es.first, es.second));
		}
		// Sanity check that there were no
		// duplicate mappings.
		assert(bimap.size() == in.size());
	}

	const char *stringify(E e) const {
		auto i = bimap.left.find(e);
		assert(i != bimap.left.end() && "Missing mapping for enumerated value");
		return i->second.c_str();
	}

	E parse(const char *s) const {
		E e;
		bool result = parse(s, &e);
		assert(result && "Missing string->enum mapping");
		return e;
	}

	bool parse(const char *s, E *e) const {
		auto i = bimap.right.find(s);
		if (i == bimap.right.end())
		{
			return false;
		}

		*e = i->second;
		return true;
	}
};

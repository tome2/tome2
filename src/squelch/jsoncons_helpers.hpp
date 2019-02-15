#pragma once

#include <boost/optional.hpp>
#include <jsoncons/json.hpp>
#include <string>

namespace squelch {

template <class T>
boost::optional<T> get_optional(jsoncons::json const &json, std::string const &key)
{
	if (!json.has_key(key))
	{
		return boost::none;
	}

	auto value = json.at(key);

	if (!value.is<T>())
	{
		return boost::none;
	}

	return value.as<T>();
}

} // namespace squelch

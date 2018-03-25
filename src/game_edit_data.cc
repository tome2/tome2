#include "game_edit_data.hpp"

#include <algorithm>

std::vector<int> const GameEditData::k_info_keys() const
{
	std::vector<int> keys;

	std::transform(std::begin(k_info),
		       std::end(k_info),
		       std::back_inserter(keys),
		       [] (auto e) { return e.first; });

	std::sort(std::begin(keys),
		  std::end(keys));

	return keys;
};

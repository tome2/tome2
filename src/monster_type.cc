#include "monster_type_fwd.hpp"
#include "monster_type.hpp"
#include "monster2.hpp"

std::shared_ptr<monster_race> monster_type::race() const
{
	return race_info_idx(r_idx, ego);
}

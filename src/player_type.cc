#include "player_type.hpp"

#include <algorithm>

bool player_type::has_ability(u16b ability_idx) const
{
	return std::find(
	        abilities.begin(),
	        abilities.end(),
	        ability_idx) != abilities.end();
}

void player_type::gain_ability(u16b ability_idx)
{
	// Duplicates don't really matter, so let's just
	// accept whatever value we get without checking
	// anything.
	abilities.push_back(ability_idx);
}

void player_type::lose_ability(u16b ability_idx)
{
	abilities.erase(
	        std::remove(
	                abilities.begin(),
	                abilities.end(),
	                ability_idx));
}

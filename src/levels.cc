#include "levels.hpp"

#include "game.hpp"
#include "variable.hpp"

static level_data const &current_level_data()
{
	static level_data *default_level_data = new level_data { };

	auto const &d_info = game->edit_data.d_info;
	auto const &level_data_by_depth = d_info[dungeon_type].level_data_by_depth;

	auto const it = level_data_by_depth.find(dun_level);
	if (it != level_data_by_depth.end())
	{
		return it->second;
	}
	else
	{
		return *default_level_data;
	}
}

int get_branch()
{
	return current_level_data().branch;
}

int get_fbranch()
{
	return current_level_data().fbranch;
}

int get_flevel()
{
	return current_level_data().flevel;
}

boost::optional<std::string> get_dungeon_save_extension()
{
	return current_level_data().save_extension;
}

boost::optional<std::string> get_dungeon_map_name()
{
	return current_level_data().map_name;
}

boost::optional<std::string> get_dungeon_name()
{
	return current_level_data().name;
}

dungeon_flag_set get_level_flags()
{
	return current_level_data().flags;
}

boost::optional<std::string> get_level_description()
{
	return current_level_data().description;
}

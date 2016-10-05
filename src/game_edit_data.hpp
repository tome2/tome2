#pragma once

#include "ability_type.hpp"
#include "dungeon_info_type.hpp"
#include "hist_type.hpp"
#include "owner_type.hpp"
#include "player_class.hpp"
#include "player_race.hpp"
#include "player_race_mod.hpp"
#include "randart_gen_type.hpp"
#include "randart_part_type.hpp"
#include "set_type.hpp"
#include "store_action_type.hpp"
#include "store_info_type.hpp"
#include "vault_type.hpp"

#include <vector>

/**
 * Game edit data, i.e. the parsed contents of the edit .txt
 * files.
 */
struct GameEditData {

	/**
	 * Dungeons
	 */
	std::vector<dungeon_info_type> d_info;

	/**
	 * Vaults
	 */
	std::vector<vault_type> v_info;

	/**
	 * Random artifact part descriptors, i.e. the bits that
	 * randarts are made up of.
	 */
	std::vector<randart_part_type> ra_info;

	/**
	 * Random artifact generation parameters.
	 */
	std::vector<randart_gen_type> ra_gen;

	/**
	 * Artifact sets
	 */
	std::vector<set_type> set_info;

	/**
	 * Building actions.
	 */
	std::vector<store_action_type> ba_info;

	/**
	 * Buildings
	 */
	std::vector<store_info_type> st_info;

	/**
	 * Building owners.
	 */
	std::vector<owner_type> ow_info;

	/**
	 * Player classes.
	 */
	std::vector<player_class> class_info;

	/**
	 * Player races.
	 */
	std::vector<player_race> race_info;

	/**
	 * Player subraces.
	 */
	std::vector<player_race_mod> race_mod_info;

	/**
	 * Player race histories
	 */
	std::vector<hist_type> bg;

	/**
	 * Base skills for all characters.
	 */
	skill_modifiers gen_skill;

	/**
	 * Player abilities.
	 */
	std::vector<ability_type> ab_info;

};

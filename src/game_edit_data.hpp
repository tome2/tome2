#pragma once

#include "hist_type.hpp"
#include "owner_type.hpp"
#include "player_class.hpp"
#include "player_race.hpp"
#include "player_race_mod.hpp"
#include "randart_gen_type.hpp"
#include "randart_part_type.hpp"
#include "store_action_type.hpp"
#include "vault_type.hpp"

#include <vector>

/**
 * Game edit data, i.e. the parsed contents of the edit .txt
 * files.
 */
struct GameEditData {

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
	 * Building actions.
	 */
	std::vector<store_action_type> ba_info;

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

};

#pragma once

#include "owner_type.hpp"
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

};

#pragma once

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

};

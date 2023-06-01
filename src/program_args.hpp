#pragma once

#include <optional>
#include <string>

/**
 * Program command line arguments.
 */
struct program_args {

	/**
	 * Wizard mode?
	 */
	bool wizard = false;

	/**
	 * Force key set?
	 */
	std::optional<char> force_key_set;

	/**
	 * Character name.
	 */
	std::string player_name;

	/**
	 * Select the given module instead of prompting
	 * the user.
	 */
	const char *module = nullptr;

};

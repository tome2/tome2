#pragma once

#include "object_flag_set.hpp"

#include <vector>

struct object_flag_meta {

	/**
	 * Flag set representation of the object flag.
	 */
	const object_flag_set flag_set;

	/**
	 * Name of the object flag.
	 */
	const char *name;

	/**
	 * Edit file name of the object flag.
	 */
	const char *e_name;

	/**
	 * Character sheet name of the object flag.
	 */
	const char *c_name;

	/**
	 * Character sheet page.
	 */
	const int c_page;

	/**
	 * Character sheet column.
	 */
	const int c_column;

	/**
	 * Character sheet row.
	 */
	const int c_row;

	/**
	 * Character sheet type.
	 */
	char c_type;

	/**
	 * Priority wrt. other flags in the same position
	 * on the character sheet.
	 */
	int c_priority;

	/**
	 * Is this flag *described* using PVAL?
	 */
	bool is_pval;

	/**
	 * Is this a flag which affects ESP?
	 */
	bool is_esp;

};

/**
 * Get a vector of all the object flags.
 */
std::vector<object_flag_meta const *> const &object_flags_meta();

/**
 * Get a flag representing all ESP flags.
 */
object_flag_set const &object_flags_esp();

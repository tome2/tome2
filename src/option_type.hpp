#pragma once

#include "h-basic.h"

/**
 * Option descriptor.
 */
struct option_type
{
	/**
	 * Address of actual option variable.
	 */
	bool *o_var;

	/**
	 * Option page number.
	 */
	byte o_page;

	/**
	 * Savefile bit in the page-specific list of options.
	 */
	byte o_bit;

	/**
	 * Textual name.
	 */
	const char *o_text;

	/**
	 * Textual description
	 */
	const char *o_desc;
};

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
	bool_ *o_var;

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
	cptr o_text;

	/**
	 * Textual description
	 */
	cptr o_desc;
};

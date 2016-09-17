#pragma once

#include "h-basic.h"

#include <string>

/**
 * Message
 */
struct message {
	/**
	 * Message color.
	 */
	byte color = 0;

	/**
	 * Repetation count for this message.
	 */
	u32b count = 0;

	/**
	 * Message text.
	 */
	std::string text;

	/**
	 * Get message text with count
	 */
	std::string text_with_count() const;

};

#pragma once

#include "h-basic.hpp"
#include "power_activation.hpp"

#include <string>

/**
 * Power descriptor.
 */
struct power_type
{
	std::string name;              /* Name */
	std::string desc_text;         /* Text describing power */
	std::string gain_text;         /* Text displayed on gaining the power */
	std::string lose_text;         /* Text displayed on losing the power */
	power_activation activation;

	power_type(
		const char *name_,
		const char *desc_text_,
		const char *gain_text_,
		const char *lose_text_,
		power_activation const &activation_)
		: name(name_)
		, desc_text(desc_text_)
		, gain_text(gain_text_)
		, lose_text(lose_text_)
		, activation(activation_)
	{
	}

};

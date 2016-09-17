#include "message.hpp"

#include <fmt/format.h>

std::string message::text_with_count() const
{
	if (count > 1)
	{
		return fmt::format("{} <{}x>", text, count);
	}
	else
	{
		return text;
	}
}

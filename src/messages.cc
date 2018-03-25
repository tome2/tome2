#include "messages.hpp"

#include "game.hpp"

#include <fmt/format.h>
#include <string>

s16b Messages::size() const
{
	return buffer.size();
}

message const &Messages::at(int age) const
{
	assert(age >= 0);
	assert(age < size());

	// Age indexes backward through history and is zero-based, so...
	std::size_t i = buffer.size() - 1 - age;

	// Get the message
	return buffer.at(i);
}

void Messages::add(const char *msg, byte color)
{
	assert(msg != nullptr);
	add(std::string(msg), color);
}

void Messages::add(std::string const &msg, byte color)
{
	// If the message is the same as the last message,
	// we just increment the counter instead of adding
	// the message.
	if ((!buffer.empty()) && (buffer.back().text == msg))
	{
		buffer.back().count += 1;
		return;
	}

	// Push onto the end of the buffer.
	message message;
	message.color = color;
	message.count = 1;
	message.text = msg;
	buffer.push_back(message);
}

void Messages::add(message const &m)
{
	buffer.push_back(m);
}

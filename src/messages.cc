#include "messages.hpp"

#include <boost/circular_buffer.hpp>
#include <fmt/format.h>
#include <string>

/*
 * OPTION: Maximum number of messages to remember (see "io.c")
 * Default: assume maximal memorization of 2048 total messages
 */
#define MESSAGE_MAX     2048

/**
 * Circular buffer for the messages
 */
static boost::circular_buffer<message> *buffer()
{
	static auto *instance = new boost::circular_buffer<message>(MESSAGE_MAX);
	return instance;
}

s16b message_num()
{
	return buffer()->size();
}

message const &message_at(int age)
{
	assert(age >= 0);
	assert(age < message_num());

	// Age indexes backward through history and is zero-based, so...
	std::size_t i = buffer()->size() - 1 - age;

	// Get the message
	return buffer()->at(i);
}

void message_add(cptr str, byte color)
{
	assert(str != nullptr);

	// Shorthand to avoid syntactic clutter
	auto buf = buffer();

	// If the message is the same as the last message,
	// we just increment the counter instead of adding
	// the message.
	if ((!buf->empty()) && (buf->back().text == str))
	{
		buf->back().count += 1;
		return;
	}

	// Push onto the end of the buffer.
	message message;
	message.color = color;
	message.count = 1;
	message.text = str;
	buf->push_back(message);
}

void message_add(message const &message)
{
	buffer()->push_back(message);
}

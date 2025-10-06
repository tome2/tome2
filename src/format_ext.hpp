#include <utility>

#pragma once

#include <fmt/format.h>
#include <string>

struct singular_prefix {

private:
	std::string m_s;

	friend std::string format_as(const singular_prefix &sp);

public:
	explicit singular_prefix(const std::string& s)
		: m_s(s)
	{
	}

	explicit singular_prefix(std::string &&s)
		: m_s(std::move(s))
	{
	}

};

//
// Formatting support for fmtlib
//
std::string format_as(const singular_prefix &sp);

// Class to simplify migration off deprecated fmt::MemoryWriter.
// My goal here was to minimize the diff necessary.
// This class should probably be removed and fmt:: (or std::format) used directly.
class fmtMemoryWriter {
private:
	fmt::memory_buffer m_buf;

public:
	template <typename... Ts> void write(Ts &&...ts) {
		fmt::format_to(std::back_inserter(m_buf), std::forward<Ts>(ts)...);
	}

	std::string str() {
		return fmt::to_string(m_buf);
	}

	std::string c_str() {
		return str();
	}
};

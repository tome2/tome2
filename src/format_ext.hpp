#include <utility>

#pragma once

#include <fmt/format.h>
#include <string>

struct singular_prefix {

private:
	std::string m_s;

	friend void format_arg(fmt::BasicFormatter<char> &formatter, const char *&format_str, const singular_prefix &sp);

public:
	explicit singular_prefix(std::string s)
		: m_s(std::move(s))
	{
	}

	explicit singular_prefix(std::string &&s)
		: m_s(std::move(s))
	{
	}

	void write(fmt::Writer &w) const;

};

//
// Formatting support for fmtlib
//
void format_arg(fmt::BasicFormatter<char> &formatter, const char *&format_str, const singular_prefix &sp);

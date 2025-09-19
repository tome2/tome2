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


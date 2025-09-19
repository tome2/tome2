#include <cassert>
#include "format_ext.hpp"

#include "util.hpp"

std::string format_as(const singular_prefix &sp)
{
	assert(!sp.m_s.empty());

	if (is_a_vowel(sp.m_s[0]))
	{
		return "an " + sp.m_s;
	}
	else
	{
		return "a " + sp.m_s;
	}
}

#include "format_ext.hpp"

#include "util.hpp"

void singular_prefix::write(fmt::Writer &w) const
{
	assert(!m_s.empty());

	if (is_a_vowel(m_s[0]))
	{
		w.write("an ");
	}
	else
	{
		w.write("a ");
	}

	w.write(m_s);
}

void format_arg(fmt::BasicFormatter<char> &formatter, const char *&format_str, const singular_prefix &sp)
{
	sp.write(formatter.writer());
}

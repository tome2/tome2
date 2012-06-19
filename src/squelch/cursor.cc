#include "tome/squelch/cursor_fwd.hpp"
#include "tome/squelch/cursor.hpp"

#include <algorithm>
#include <cassert>

#include "tome/squelch/condition.hpp"

namespace squelch {

bool Cursor::is_selected(Condition const *condition) const
{
	return std::end(m_conditions) !=
		std::find(std::begin(m_conditions),
			  std::end(m_conditions),
			  condition);
}

Condition *Cursor::pop()
{
	assert(!m_conditions.empty());
	Condition *c = m_conditions.back();
	m_conditions.pop_back();
	return c;
}

Condition *Cursor::current()
{
	assert(!m_conditions.empty());
	return m_conditions.back();
}

void Cursor::move_right()
{
	// Go right if the currently selected condition has children.
	std::shared_ptr<Condition> c = current()->first_child();
	if (c)
	{
		push(c.get());
	}
}

void Cursor::move_left()
{
	if (size() > 1)
	{
		pop();
	}
}

void Cursor::move_up()
{
	if (size() > 1)
	{
		Condition *prev_top = pop();

		// Find previous child
		std::shared_ptr<Condition> prev_condition =
			current()->previous_child(prev_top);

		// Do we have a previous child?
		if (prev_condition)
		{
			push(prev_condition.get());
		}
		else
		{
			push(prev_top);
		}
	}
}

void Cursor::move_down()
{
	if (size() > 1)
	{
		Condition *prev_top = pop();

		std::shared_ptr<Condition> next_condition =
			current()->next_child(prev_top);

		if (next_condition)
		{
			push(next_condition.get());
		}
		else
		{
			push(prev_top);
		}
	}
}

} // namespace

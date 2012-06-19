#ifndef H_8a10111d_64a1_4af9_a85b_24ec8922d3fa
#define H_8a10111d_64a1_4af9_a85b_24ec8922d3fa

#include <boost/noncopyable.hpp>
#include <deque>

#include "tome/squelch/condition_fwd.hpp"

namespace squelch {

/**
 * Cursor which maintains selected condition(s)
 */
class Cursor : public boost::noncopyable
{
public:
	bool is_selected(Condition const *condition) const;

	void push(Condition *condition) {
		m_conditions.push_back(condition);
	}

	Condition *pop();

	Condition *current();

	void clear() {
		m_conditions.clear();
	}

	bool empty() const {
		return m_conditions.empty();
	}

	std::size_t size() const {
		return m_conditions.size();
	}

	void move_left();
	void move_right();
	void move_up();
	void move_down();

private:
	std::deque<Condition *> m_conditions;
};

} // namespace

#endif

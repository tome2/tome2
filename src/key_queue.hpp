#pragma once

#include "key_queue_fwd.hpp"

#include <boost/circular_buffer.hpp>


/*
 * Low-level key queue handling.
 */
class key_queue {

private:
	boost::circular_buffer<char> m_buffer;

public:
	/*
	 * Overflow signal
	 */
	enum class push_result_t {
		OK,
		OVERFLOW,
	};

public:
	explicit key_queue(std::size_t k)
		: m_buffer(k)
	{
	}

	void clear()
	{
		m_buffer.clear();
	}

	void push_back(char c)
	{
		if (m_buffer.full())
		{
			return; // Ignore
		}

		m_buffer.push_back(c);
	}

	push_result_t push_front(char k)
	{
		if (m_buffer.full())
		{
			return push_result_t::OVERFLOW;
		}

		m_buffer.push_front(k);
		return push_result_t::OK;
	}

	char front() const
	{
		assert(!empty());
		return m_buffer.front();
	}

	char pop_front()
	{
		assert(!empty());
		auto ch = m_buffer.front();
		m_buffer.pop_front();
		return ch;
	}

	bool empty() const
	{
		return m_buffer.empty();
	}

};

#pragma once

#include "h-basic.h"

#include <functional>

/*
 * Timer descriptor and runtime data.
 */
struct timer_type
{
private:
	std::function<void ()> m_callback;

public:
	//
	// XXX Currently need public access for loading and saving.
	//
	bool m_enabled;
	s32b m_delay = 0;
	s32b m_countdown = 0;

public:
	/**
	 * Create a new timer
	 */
	timer_type(std::function<void()> callback, s32b delay)
		: m_callback(callback)
		, m_enabled(false)
		, m_delay(delay)
		, m_countdown(delay)
	{
	}

	timer_type(timer_type const &other) = delete;
	timer_type &operator =(timer_type const &other) = delete;

	/**
	 * Enable the timer.
	 */
	void enable()
	{
		m_enabled = true;
	}

	/**
	 * Disable the timer.
	 */
	void disable()
	{
		m_enabled = false;
	}

	/**
	 * Change delay and reset.
	 */
	void set_delay_and_reset(s32b delay)
	{
		m_delay = delay;
		m_countdown = delay;
	}

	/**
	 * Count down.
	 */
	void count_down()
	{
		if (!m_enabled)
		{
			return;
		}

		m_countdown--;
		if (m_countdown <= 0)
		{
			m_countdown = m_delay;
			m_callback();
		}
	}

};

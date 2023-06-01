#pragma once

/**
 * Taken from
 *
 *   https://github.com/milleniumbug/wiertlo/blob/master/include/wiertlo/unique_handle.hpp
 *
 */

#include <utility>

template<typename Policy>
class unique_handle
{
	typename Policy::handle_type h;

public:
	unique_handle(const unique_handle&) = delete;

	typename Policy::handle_type get() const
	{
		return h;
	}

	typename Policy::handle_type release()
	{
		typename Policy::handle_type temp = h;
		h = Policy::get_null();
		return temp;
	}

	explicit operator bool() const
	{
		return !Policy::is_null(h);
	}

	bool operator!() const
	{
		return !static_cast<bool>(*this);
	}

	void reset(typename Policy::handle_type new_handle)
	{
		typename Policy::handle_type old_handle = h;
		h = new_handle;
		if(!Policy::is_null(old_handle))
		{
			Policy::close(old_handle);
		}
	}

	void swap(unique_handle& other)
	{
		std::swap(this->h, other.h);
	}

	void reset()
	{
		reset(Policy::get_null());
	}

	~unique_handle()
	{
		reset();
	}

	unique_handle& operator=(unique_handle other) noexcept
	{
		this->swap(other);
		return *this;
	}

	unique_handle(unique_handle&& other) noexcept
	{
		this->h = other.h;
		other.h = Policy::get_null();
	}

	unique_handle()
	{
		h = Policy::get_null();
	}

	unique_handle(typename Policy::handle_type handle)
	{
		h = handle;
	}
};

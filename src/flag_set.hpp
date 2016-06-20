#pragma once

#include <array>
#include <cassert>
#include <cstdint>

#include "tome/pp/global_constexpr.hpp"

/**
 * Set of binary flags.
 */
template<std::size_t Tiers> struct flag_set
{
private:
	static constexpr std::size_t tiers = Tiers;
	std::uint32_t m_data[tiers];

public:
	static constexpr const std::size_t nbits = tiers * 32;

public:

	constexpr flag_set()
		: m_data { 0 }
	{
		// It is *extremely* important that there are absolutely
		// NO dependencies on any global objects in here; lest we
		// fall into SIOF territory; see DECLARE_FLAG_ZERO_IMPL
	}

	// This method is a workaround for a a segmentation fault
	// when compiling with GCC 5.3.0 (Arch Linux). We should be
	// able to use make() directly in DECLARE_FLAG_MAKE_INIT,
	// but GCC segfaults.
	template<std::uint32_t tier, std::size_t index> static constexpr flag_set make_static()
	{
		static_assert(tier < tiers, "tier >= tiers");
		static_assert(index < 32, "index >= 32");
		flag_set f;
		f.m_data[tier] = (1UL << index);
		return f;
	}

	static constexpr flag_set make(std::uint32_t tier, std::size_t index)
	{
		assert(tier < tiers);
		assert(index < 32);
		flag_set f;
		f.m_data[tier] = (1UL << index);
		return f;
	}

	constexpr std::size_t size() const
	{
		return tiers;
	}

	constexpr bool empty() const
	{
		for (std::size_t i = 0; i < tiers; i++)
		{
			if (m_data[i])
			{
				return false;
			}
		}
		return true;
	}

	constexpr std::size_t count() const
	{
		std::size_t n = 0;
		for (std::size_t i = 0; i < nbits; i++)
		{
			if (bit(i))
			{
				n += 1;
			}
		}
		return n;
	}

	uint32_t &operator[](std::size_t i)
	{
		assert(i < tiers);
		return m_data[i];
	}

	constexpr uint32_t const &operator [](std::size_t i) const
	{
		assert(i < tiers);
		return m_data[i];
	}

	constexpr operator bool() const
	{
		return !empty();
	}

	constexpr bool bit(std::size_t i) const
	{
		assert(i < nbits);
		return (m_data[i / 32] & (1UL << (i % 32)));
	}

	flag_set &operator |= (flag_set const &other)
	{
		for (std::size_t i = 0; i < tiers; i++)
		{
			m_data[i] |= other.m_data[i];
		}
		return *this;
	}

	constexpr flag_set operator | (flag_set const &other) const
	{
		flag_set f;
		for (std::size_t i = 0; i < tiers; i++)
		{
			f.m_data[i] = m_data[i] | other.m_data[i];
		}
		return f;
	}

	flag_set &operator &= (flag_set const &other)
	{
		for (std::size_t i = 0; i < tiers; i++)
		{
			m_data[i] &= other.m_data[i];
		}
		return *this;
	}

	constexpr flag_set operator & (flag_set const &other) const
	{
		flag_set f;
		for (std::size_t i = 0; i < tiers; i++)
		{
			f.m_data[i] = m_data[i] & other.m_data[i];
		}
		return f;
	}

	constexpr flag_set operator ~ () const
	{
		flag_set f;
		for (std::size_t i = 0; i < tiers; i++)
		{
			f.m_data[i] = ~m_data[i];
		}
		return f;
	}

};

// Implementation details, because preprocessor.
#define DECLARE_FLAG_MAKE_INIT(type, tier, index) \
   type::make_static<tier-1,index>()

/**
 * Macro for declaring a "flag" constant.
 */
#define DECLARE_FLAG(type, name, tier, index) \
  PP_GLOBAL_CONSTEXPR_CONST(type, name, DECLARE_FLAG_MAKE_INIT(type, tier, index))

/**
 * Macro for declaring a zero'ed "flag" variable.
 */
#define DECLARE_FLAG_ZERO_INTF(type, name) \
  extern type name

/**
 * Macro for declaring the implementation of a zero'ed "flag" variable.
 */
#define DECLARE_FLAG_ZERO_IMPL(type, name) \
  type name { };

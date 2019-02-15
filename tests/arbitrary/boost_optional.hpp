#pragma once

#include <boost/optional.hpp>
#include <boost/optional/optional_io.hpp>
#include <cppqc/Arbitrary.h>

namespace cppqc {

template<typename T>
boost::optional<T> arbitraryBoostOptional(RngEngine &rng, std::size_t size)
{
	std::uniform_int_distribution<> distribution(0, 4);

	if (distribution(rng) == 0)
	{
		return boost::none;
	}
	else
	{
		return Arbitrary<T>::unGen(rng, size);
	}
}

template<typename T>
std::vector<boost::optional<T>> shrinkBoostOptional(boost::optional<T> shrinkInput)
{
	std::vector<boost::optional<T>> result;

	if (shrinkInput)
	{
		result.push_back(boost::none);

		for (auto const &t: Arbitrary<T>::shrink(*shrinkInput))
		{
			result.push_back(t);
		}
	}

	return result;
}

template <typename T>
class ArbitraryImpl<boost::optional<T>> {

public:
	static const typename Arbitrary<boost::optional<T>>::unGenType unGen;

	static const typename Arbitrary<boost::optional<T>>::shrinkType shrink;

};

template <typename T>
const typename Arbitrary<boost::optional<T>>::unGenType
ArbitraryImpl<boost::optional<T>>::unGen = arbitraryBoostOptional<T>;

template <typename T>
const typename Arbitrary<boost::optional<T>>::shrinkType
ArbitraryImpl<boost::optional<T>>::shrink = shrinkBoostOptional<T>;

} // namespace cppqc

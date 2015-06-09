#pragma once

#include <type_traits>

/*
 * Make an array of a POD type.
 */
template <typename T> T *make_array(std::size_t n) {
	static_assert(std::is_pod<T>::value, "Type parameter must be POD type");
	T *array = new T[n];
	memset(array, 0, n*sizeof(T));
	return array;
}

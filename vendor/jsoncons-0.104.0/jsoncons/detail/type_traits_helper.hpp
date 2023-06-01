// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_TYPETRAITSHELPER_HPP
#define JSONCONS_DETAIL_TYPETRAITSHELPER_HPP

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <iomanip>
#include <cstdlib>
#include <cmath>
#include <cstdarg>
#include <locale>
#include <limits> 
#include <type_traits>
#include <algorithm>
#include <memory>
#include <iterator>
#include <exception>
#include <array>
#include <initializer_list>
#include <jsoncons/jsoncons_config.hpp>
#include <jsoncons/json_exception.hpp>
#include <jsoncons/detail/obufferedstream.hpp>

namespace jsoncons
{
// static_max

template <size_t arg1, size_t ... argn>
struct static_max;

template <size_t arg>
struct static_max<arg>
{
    static const size_t value = arg;
};

template <size_t arg1, size_t arg2, size_t ... argn>
struct static_max<arg1,arg2,argn ...>
{
    static const size_t value = arg1 >= arg2 ? 
        static_max<arg1,argn...>::value :
        static_max<arg2,argn...>::value; 
};

// type_wrapper

template <class T>
struct type_wrapper
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

template <class T>
struct type_wrapper<const T>
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

template <class T>
struct type_wrapper<T&>
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

template <class T>
struct type_wrapper<const T&>
{
    typedef T* pointer_type;
    typedef const T* const_pointer_type;
    typedef T value_type;
    typedef T& reference;
    typedef const T& const_reference;
};

// json_literals

namespace detail {
JSONCONS_DEFINE_LITERAL(null_literal,"null")
JSONCONS_DEFINE_LITERAL(true_literal,"true")
JSONCONS_DEFINE_LITERAL(false_literal,"false")
}

inline
unsigned char to_hex_character(unsigned char c)
{
    JSONCONS_ASSERT(c <= 0xF);

    return (c < 10) ? ('0' + c) : ('A' - 10 + c);
}

inline
bool is_control_character(uint32_t c)
{
    return c <= 0x1F || c == 0x7f;
}

inline
bool is_non_ascii_codepoint(uint32_t cp)
{
    return cp >= 0x80;
}

template <typename T>
struct is_stateless
 : public std::integral_constant<bool,  
      (std::is_default_constructible<T>::value &&
      std::is_empty<T>::value)>
{};

// type traits extensions


namespace detail {

// to_plain_pointer

template<class Pointer> inline
typename std::pointer_traits<Pointer>::element_type* to_plain_pointer(Pointer ptr)
{       
    return (std::addressof(*ptr));
}

template<class T> inline
T * to_plain_pointer(T * ptr)
{       
    return (ptr);
}  

// is_string_like

template <class T, class Enable=void>
struct is_string_like : std::false_type {};

template <class T>
struct is_string_like<T, 
                      typename std::enable_if<!std::is_void<typename T::traits_type>::value
>::type> : std::true_type {};

// is_integer_like

template <class T, class Enable=void>
struct is_integer_like : std::false_type {};

template <class T>
struct is_integer_like<T, 
                       typename std::enable_if<std::is_integral<T>::value && 
                       std::is_signed<T>::value && 
                       !std::is_same<T,bool>::value>::type> : std::true_type {};

// is_uinteger_like

template <class T, class Enable=void>
struct is_uinteger_like : std::false_type {};

template <class T>
struct is_uinteger_like<T, 
                        typename std::enable_if<std::is_integral<T>::value && 
                        !std::is_signed<T>::value && 
                        !std::is_same<T,bool>::value>::type> : std::true_type {};

// is_floating_point_like

template <class T, class Enable=void>
struct is_floating_point_like : std::false_type {};

template <class T>
struct is_floating_point_like<T, 
                              typename std::enable_if<std::is_floating_point<T>::value>::type> : std::true_type {};

// is_map_like

template <class T, class Enable=void>
struct is_map_like : std::false_type {};

template <class T>
struct is_map_like<T, 
                   typename std::enable_if<!std::is_void<typename T::mapped_type>::value>::type> 
    : std::true_type {};

// is_array_like
template<class T>
struct is_array_like : std::false_type {};

template<class E, size_t N>
struct is_array_like<std::array<E, N>> : std::true_type {};

// is_vector_like

template <class T, class Enable=void>
struct is_vector_like : std::false_type {};

template <class T>
struct is_vector_like<T, 
                      typename std::enable_if<!std::is_void<typename T::value_type>::value &&
                                              !is_array_like<T>::value && 
                                              !is_string_like<T>::value && 
                                              !is_map_like<T>::value 
>::type> 
    : std::true_type {};

}

}

#endif

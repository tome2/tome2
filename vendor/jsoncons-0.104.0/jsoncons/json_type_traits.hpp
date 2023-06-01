// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_JSON_TYPE_TRAITS_HPP
#define JSONCONS_JSON_TYPE_TRAITS_HPP

#include <array>
#include <string>
#include <vector>
#include <valarray>
#include <exception>
#include <cstdlib>
#include <cstring>
#include <utility>
#include <algorithm>
#include <fstream>
#include <limits>
#include <type_traits>
#include <jsoncons/jsoncons_utilities.hpp>
#include <jsoncons/detail/type_traits_helper.hpp>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"
#endif

namespace jsoncons {

// null_type

struct null_type
{
};

// json_type_traits

template <class Json, class T, class Enable=void>
struct json_type_traits
{
    static const bool is_compatible = false;

    static bool is(const Json&)
    {
        return false;
    }
};

namespace detail {

// is_incompatible
template<class Json, class T, class Enable = void>
struct is_incompatible : std::false_type {};


// is_incompatible
template<class Json, class T>
struct is_incompatible<Json,T,
    typename std::enable_if<!std::integral_constant<bool, json_type_traits<Json, T>::is_compatible>::value>::type
> : std::true_type {};

// is_compatible_string_type
template<class Json, class T, class Enable=void>
struct is_compatible_string_type : std::false_type {};

template<class Json, class T>
struct is_compatible_string_type<Json,T, 
    typename std::enable_if<!std::is_same<T,typename Json::array>::value &&
    detail::is_string_like<T>::value && 
    !is_incompatible<Json,typename std::iterator_traits<typename T::iterator>::value_type>::value
>::type> : std::true_type {};

// is_compatible_array_type
template<class Json, class T, class Enable=void>
struct is_compatible_array_type : std::false_type {};

template<class Json, class T>
struct is_compatible_array_type<Json,T, 
    typename std::enable_if<!std::is_same<T,typename Json::array>::value &&
    detail::is_vector_like<T>::value && 
    !is_incompatible<Json,typename std::iterator_traits<typename T::iterator>::value_type>::value
>::type> : std::true_type {};

// is_compatible_object_type
template<class Json, class T, class Enable=void>
struct is_compatible_object_type : std::false_type {};

template<class Json, class T>
struct is_compatible_object_type<Json,T, 
                       typename std::enable_if<
    !is_incompatible<Json,typename T::mapped_type>::value
>::type> : std::true_type {};

// is_std_array
template<class T>
struct is_std_array : std::false_type {};

template<class E, size_t N>
struct is_std_array<std::array<E, N>> : std::true_type {};

template <class Json, class T>
class json_array_input_iterator
{
public:
    typedef typename Json::const_array_iterator iterator_base;
    typedef typename std::iterator_traits<iterator_base>::value_type value_type;
    typedef typename std::iterator_traits<iterator_base>::difference_type difference_type;
    typedef typename std::iterator_traits<iterator_base>::pointer pointer;
    typedef T reference;
    typedef std::input_iterator_tag iterator_category;

    json_array_input_iterator()
    {
    }

    json_array_input_iterator(iterator_base it)
        : it_(it)
    {
    }

    json_array_input_iterator& operator=(json_array_input_iterator rhs)
    {
        swap(*this,rhs);
        return *this;
    }

    json_array_input_iterator& operator++()
    {
        ++it_;
        return *this;
    }

    json_array_input_iterator operator++(int) // postfix increment
    {
        json_array_input_iterator temp(*this);
        ++it_;
        return temp;
    }

    json_array_input_iterator& operator--()
    {
        --it_;
        return *this;
    }

    json_array_input_iterator operator--(int)
    {
        json_array_input_iterator temp(*this);
        --it_;
        return temp;
    }

    reference operator*() const
    {
        return json_type_traits<Json,T>::as(*it_);
    }

    friend bool operator==(const json_array_input_iterator& it1, const json_array_input_iterator& it2)
    {
        return it1.it_ == it2.it_;
    }
    friend bool operator!=(const json_array_input_iterator& it1, const json_array_input_iterator& it2)
    {
        return !(it1.it_ == it2.it_);
    }
    friend void swap(json_array_input_iterator& lhs, json_array_input_iterator& rhs)
    {
        using std::swap;
        swap(lhs.it_,rhs.it_);
        swap(lhs.empty_,rhs.empty_);
    }

private:
    iterator_base it_;
};

template <class Json, class T>
class json_object_input_iterator
{
public:
    typedef typename Json::const_object_iterator iterator_base;
    typedef typename std::iterator_traits<iterator_base>::value_type value_type;
    typedef typename std::iterator_traits<iterator_base>::difference_type difference_type;
    typedef typename std::iterator_traits<iterator_base>::pointer pointer;
    typedef T reference;
    typedef std::input_iterator_tag iterator_category;
    typedef typename T::first_type key_type;
    typedef typename T::second_type mapped_type;

    json_object_input_iterator()
    {
    }

    json_object_input_iterator(iterator_base it)
        : it_(it)
    {
    }

    json_object_input_iterator& operator=(json_object_input_iterator rhs)
    {
        swap(*this,rhs);
        return *this;
    }

    json_object_input_iterator& operator++()
    {
        ++it_;
        return *this;
    }

    json_object_input_iterator operator++(int) // postfix increment
    {
        json_object_input_iterator temp(*this);
        ++it_;
        return temp;
    }

    json_object_input_iterator& operator--()
    {
        --it_;
        return *this;
    }

    json_object_input_iterator operator--(int)
    {
        json_object_input_iterator temp(*this);
        --it_;
        return temp;
    }

    reference operator*() const
    {
        return T(key_type(it_->key()),json_type_traits<Json,mapped_type>::as(it_->value()));
    }

    friend bool operator==(const json_object_input_iterator& it1, const json_object_input_iterator& it2)
    {
        return it1.it_ == it2.it_;
    }
    friend bool operator!=(const json_object_input_iterator& it1, const json_object_input_iterator& it2)
    {
        return !(it1.it_ == it2.it_);
    }
    friend void swap(json_object_input_iterator& lhs, json_object_input_iterator& rhs)
    {
        using std::swap;
        swap(lhs.it_,rhs.it_);
        swap(lhs.empty_,rhs.empty_);
    }

private:
    iterator_base it_;
};

}

template<class Json>
struct json_type_traits<Json, typename type_wrapper<typename Json::char_type>::const_pointer_type>
{
    typedef typename Json::char_type char_type;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        return j.is_string();
    }
    static const char_type* as(const Json& j)
    {
        return j.as_cstring();
    }
    template <class ... Args>
    static Json to_json(Args&&... args)
    {
        return Json(typename Json::variant(std::forward<Args>(args)...));
    }
};

template<class Json>
struct json_type_traits<Json, typename type_wrapper<typename Json::char_type>::pointer_type>
{
    typedef typename Json::char_type char_type;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        return j.is_string();
    }
    template <class ... Args>
    static Json to_json(Args&&... args)
    {
        return Json(typename Json::variant(std::forward<Args>(args)...));
    }
};

// integral

template<class Json, class T>
struct json_type_traits<Json, T,
                        typename std::enable_if<detail::is_integer_like<T>::value
>::type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        if (j.is_integer())
        {
            return j.as_integer() >= (std::numeric_limits<T>::min)() && j.as_integer() <= (std::numeric_limits<T>::max)();
        }
        else if (j.is_uinteger())
        {
            return j.as_uinteger() <= static_cast<uint64_t>((std::numeric_limits<T>::max)());
        }
        else
        {
            return false;
        }
    }
    static T as(const Json& j)
    {
        return static_cast<T>(j.as_integer());
    }
    template <class ... Args>
    static Json to_json(Args&&... args)
    {
        return Json::from_integer(std::forward<Args>(args)...);
    }
};

template<class Json, class T>
struct json_type_traits<Json, T,
                        typename std::enable_if<detail::is_uinteger_like<T>::value
>::type >
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        if (j.is_integer())
        {
            return j.as_integer() >= 0 && static_cast<uint64_t>(j.as_integer()) <= (std::numeric_limits<T>::max)();
        }
        else if (j.is_uinteger())
        {
            return j.as_uinteger() <= (std::numeric_limits<T>::max)();
        }
        else
        {
            return false;
        }
    }
    static T as(const Json& j)
    {
        return static_cast<T>(j.as_uinteger());
    }

    template <class ... Args>
    static Json to_json(Args&&... args)
    {
        return Json::from_uinteger(std::forward<Args>(args)...);
    }
};

template<class Json,class T>
struct json_type_traits<Json, T,
                        typename std::enable_if<std::is_floating_point<T>::value
>::type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        return j.is_double();
    }
    static T as(const Json& j)
    {
        return static_cast<T>(j.as_double());
    }
    template <class ... Args>
    static Json to_json(Args&&... args)
    {
        return Json::from_floating_point(std::forward<Args>(args)...);
    }
};

template<class Json>
struct json_type_traits<Json, typename Json::object>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        return j.is_object();
    }
    template <class ... Args>
    static Json to_json(Args&&... args)
    {
        return Json(typename Json::variant(std::forward<Args>(args)...));
    }
};

template<class Json>
struct json_type_traits<Json, typename Json::array>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        return j.is_array();
    }
    template <class ... Args>
    static Json to_json(Args&&... args)
    {
        return Json(typename Json::variant(std::forward<Args>(args)...));
    }
};

template<class Json>
struct json_type_traits<Json, Json>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json&) JSONCONS_NOEXCEPT
    {
        return true;
    }
    static Json as(Json j)
    {
        return j;
    }
    static Json to_json(const Json& val)
    {
        return val;
    }
    static Json to_json(const Json& val, allocator_type)
    {
        return val;
    }
};

template<class Json>
struct json_type_traits<Json, jsoncons::null_type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        return j.is_null();
    }
    static typename jsoncons::null_type as(const Json& j)
    {
        JSONCONS_ASSERT(j.is_null());
        return jsoncons::null_type();
    }
    static Json to_json(jsoncons::null_type)
    {
        return Json::null();
    }
    static Json to_json(jsoncons::null_type, allocator_type)
    {
        return Json::null();
    }
};

template<class Json>
struct json_type_traits<Json, bool>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        return j.is_bool();
    }
    static bool as(const Json& j)
    {
        return j.as_bool();
    }
    template <class ... Args>
    static Json to_json(Args&&... args)
    {
        return Json(typename Json::variant(std::forward<Args>(args)...));
    }
};

template<class Json,class T>
struct json_type_traits<Json, T, typename std::enable_if<std::is_same<T, 
    std::conditional<!std::is_same<bool,std::vector<bool>::const_reference>::value,
                     std::vector<bool>::const_reference,
                     void>::type>::value>::type>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        return j.is_bool();
    }
    static bool as(const Json& j)
    {
        return j.as_bool();
    }
    template <class ... Args>
    static Json to_json(Args&&... args)
    {
        return Json(typename Json::variant(std::forward<Args>(args)...));
    }
};

template<class Json>
struct json_type_traits<Json, std::vector<bool>::reference>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        return j.is_bool();
    }
    static bool as(const Json& j)
    {
        return j.as_bool();
    }
    template <class ... Args>
    static Json to_json(Args&&... args)
    {
        return Json(typename Json::variant(std::forward<Args>(args)...));
    }
};

template<class Json, typename T>
struct json_type_traits<Json, T, 
                        typename std::enable_if<detail::is_compatible_array_type<Json,T>::value && 
                                                !detail::is_std_array<T>::value>::type>
{
    typedef typename std::iterator_traits<typename T::iterator>::value_type element_type;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        bool result = j.is_array();
        if (result)
        {
            for (auto e : j.array_range())
            {
                if (!e.template is<element_type>())
                {
                    result = false;
                    break;
                }
            }
        }
        return result;
    }

    template <class Ty = element_type>
    static typename std::enable_if<!(std::is_integral<Ty>::value && !std::is_same<Ty,bool>::value),T>::type
    as(const Json& j)
    {
        if (j.is_array())
        {
            T v(detail::json_array_input_iterator<Json, element_type>(j.array_range().begin()),
                detail::json_array_input_iterator<Json, element_type>(j.array_range().end()));
            return v;
        }
        else
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Attempt to cast json non-array to array"));
        }
    }

    template <class Ty = element_type>
    static typename std::enable_if<std::is_integral<Ty>::value && !std::is_same<Ty,bool>::value,T>::type
    as(const Json& j)
    {
        if (j.is_array())
        {
            T v(detail::json_array_input_iterator<Json, element_type>(j.array_range().begin()),
                detail::json_array_input_iterator<Json, element_type>(j.array_range().end()));
            return v;
        }
        else if (j.is_byte_string())
        {
            T v(j.as_byte_string_view().begin(),j.as_byte_string_view().end());
            return v;
        }
        else
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Attempt to cast json non-array to array"));
        }
    }

    static Json to_json(const T& val)
    {
        Json j = typename Json::array();
        auto first = std::begin(val);
        auto last = std::end(val);
        size_t size = std::distance(first,last);
        j.reserve(size);
        for (auto it = first; it != last; ++it)
        {
            j.push_back(*it);
        }
        return j;
    }

    static Json to_json(const T& val, const allocator_type& allocator)
    {
        Json j = typename Json::array(allocator);
        auto first = std::begin(val);
        auto last = std::end(val);
        size_t size = std::distance(first, last);
        j.reserve(size);
        for (auto it = first; it != last; ++it)
        {
            j.push_back(*it);
        }
        return j;
    }
};

template<class Json, typename T>
struct json_type_traits<Json, T, 
                        typename std::enable_if<detail::is_compatible_string_type<Json,T>::value>::type>
{
    typedef typename std::iterator_traits<typename T::iterator>::value_type element_type;
    typedef typename Json::allocator_type allocator_type;
	typedef typename T::allocator_type string_allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        return j.is_string();
    }

    static T as(const Json& j)
    {
        if (j.is_string())
        {
			return j.as_string(string_allocator_type());
        }
        else
        {
            T s;
            j.dump(s);
            return s;
        }
    }

    static Json to_json(const T& val)
    {
        return Json(typename Json::variant(val.data(), val.size()));
    }

    static Json to_json(const T& val, const allocator_type& allocator)
    {
        return Json(typename Json::variant(val.data(),val.size(),allocator));
    }
};

template<class Json, typename T>
struct json_type_traits<Json, T, 
                        typename std::enable_if<detail::is_compatible_object_type<Json,T>::value>::type
>
{
    typedef typename T::mapped_type mapped_type;
    typedef typename T::value_type value_type;
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        bool result = j.is_object();
        for (auto member : j.object_range())
        {
            if (!member.value().template is<mapped_type>())
            {
                result = false;
            }
        }
        return result;
    }

    static T as(const Json& j)
    {
        T v(detail::json_object_input_iterator<Json,value_type>(j.object_range().begin()),
            detail::json_object_input_iterator<Json,value_type>(j.object_range().end()));
        return v;
    }

    static Json to_json(const T& val)
    {
        Json j;
        j.reserve(val.size());
        for (auto p: val)
        {
            j.set(p.first, p.second);
        }
        return j;
    }

    static Json to_json(const T& val, const allocator_type& allocator)
    {
        Json j(allocator);
        j.reserve(val.size());
        for (auto p: val)
        {
            j.set(p.first, p.second);
        }
        return j;
    }
};

template<class Json, class E, size_t N>
struct json_type_traits<Json, std::array<E, N>>
{
    typedef typename Json::allocator_type allocator_type;

    typedef E element_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        bool result = j.is_array() && j.size() == N;
        if (result)
        {
            for (auto e : j.array_range())
            {
                if (!e.template is<element_type>())
                {
                    result = false;
                    break;
                }
            }
        }
        return result;
    }

    static std::array<E, N> as(const Json& j)
    {
        std::array<E, N> buff;
        JSONCONS_ASSERT(j.size() == N);
        for (size_t i = 0; i < N; i++)
        {
            buff[i] = j[i].template as<E>();
        }
        return buff;
    }

    static Json to_json(const std::array<E, N>& val)
    {
        Json j = typename Json::array();
        j.reserve(N);
        for (auto it = val.begin(); it != val.end(); ++it)
        {
            j.push_back(*it);
        }
        return j;
    }

    static Json to_json(const std::array<E, N>& val, 
                        const allocator_type& allocator)
    {
        Json j = typename Json::array(allocator);
        j.reserve(N);
        for (auto it = val.begin(); it != val.end(); ++it)
        {
            j.push_back(*it);
        }
        return j;
    }
};

namespace detail
{

template<size_t Pos, class Json, class Tuple>
struct json_tuple_helper
{
    using element_type = typename std::tuple_element<Pos - 1, Tuple>::type;
    using next = json_tuple_helper<Pos - 1, Json, Tuple>;
    
    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        if(j[Pos - 1].template is<element_type>())
        {
            return next::is(j);
        }
        else
        {
            return false;
        }
    }

    static void as(Tuple& tuple, const Json& j)
    {
        std::get<Pos - 1>(tuple) = j[Pos - 1].template as<element_type>();
        next::as(tuple, j);
    }

    static void to_json(const Tuple& tuple, std::array<Json, std::tuple_size<Tuple>::value>& jsons)
    {
        jsons[Pos - 1] = json_type_traits<Json, element_type>::to_json(std::get<Pos-1>(tuple));
        next::to_json(tuple, jsons);
    }
};

template<class Json, class Tuple>
struct json_tuple_helper<0, Json, Tuple>
{
    static bool is(const Json&) JSONCONS_NOEXCEPT
    {
        return true;
    }

    static void as(Tuple&, const Json&)
    {
    }

    static void to_json(const Tuple&, std::array<Json, std::tuple_size<Tuple>::value>&)
    {
    }
};

}

template<class Json, typename... E>
struct json_type_traits<Json, std::tuple<E...>>
{
private:
    using helper = detail::json_tuple_helper<sizeof...(E), Json, std::tuple<E...>>;

public:
    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        return helper::is(j);
    }
    
    static std::tuple<E...> as(const Json& j)
    {
        std::tuple<E...> buff;
        helper::as(buff, j);
        return buff;
    }
    
    static Json to_json(const std::tuple<E...>& val)
    {
        std::array<Json, sizeof...(E)> buf;
        helper::to_json(val, buf);
        return Json(typename Json::array(buf.begin(), buf.end()));
    }
};

template<class Json, class T1, class T2>
struct json_type_traits<Json, std::pair<T1,T2>>
{
public:
    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        return j.is_array() && j.size() == 2;
    }
    
    static std::pair<T1,T2> as(const Json& j)
    {
        return std::make_pair<T1,T2>(j[0]. template as<T1>(),j[1]. template as<T2>());
    }
    
    static Json to_json(const std::pair<T1,T2>& val)
    {
        return typename Json::array{val.first,val.second};
    }
};

template<class Json, class Allocator>
struct json_type_traits<Json, basic_byte_string<Allocator>>
{
public:
    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        return j.is_byte_string();
    }
    
    static basic_byte_string<Allocator> as(const Json& j)
    {
        return basic_byte_string<Allocator>(j.as_byte_string_view());
    }
    
    static Json to_json(const basic_byte_string<Allocator>& val)
    {
        return Json(val.data(),val.length());
    }
};

// std::valarray

template<class Json, class T>
struct json_type_traits<Json, std::valarray<T>>
{
    typedef typename Json::allocator_type allocator_type;

    static bool is(const Json& j) JSONCONS_NOEXCEPT
    {
        bool result = j.is_array();
        if (result)
        {
            for (auto e : j.array_range())
            {
                if (!e.template is<T>())
                {
                    result = false;
                    break;
                }
            }
        }
        return result;
    }
    
    static std::valarray<T> as(const Json& j)
    {
        if (j.is_array())
        {
            std::valarray<T> v(j.size());
            for (size_t i = 0; i < j.size(); ++i)
            {
                v[i] = j[i].template as<T>();
            }
            return v;
        }
        else
        {
            JSONCONS_THROW(json_exception_impl<std::runtime_error>("Attempt to cast json non-array to array"));
        }
    }
    
    static Json to_json(const std::valarray<T>& val)
    {
        Json j = typename Json::array();
        auto first = std::begin(val);
        auto last = std::end(val);
        size_t size = std::distance(first,last);
        j.reserve(size);
        for (auto it = first; it != last; ++it)
        {
            j.push_back(*it);
        }
        return j;
    }

    static Json to_json(const std::valarray<T>& val, const allocator_type& allocator)
    {
        Json j = typename Json::array(allocator);
        auto first = std::begin(val);
        auto last = std::end(val);
        size_t size = std::distance(first,last);
        j.reserve(size);
        for (auto it = first; it != last; ++it)
        {
            j.push_back(*it);
        }
        return j;
    }
};

}

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

#endif

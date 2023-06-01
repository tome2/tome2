// Copyright 2013 Daniel Parker
// Distributed under the Boost license, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

// See https://github.com/danielaparker/jsoncons for latest version

#ifndef JSONCONS_DETAIL_NUMBERPARSERS_HPP
#define JSONCONS_DETAIL_NUMBERPARSERS_HPP

#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>
#include <istream>
#include <ostream>
#include <iomanip>
#include <cstdlib>
#include <cstdarg>
#include <locale>
#include <limits> 
#include <type_traits>
#include <algorithm>
#include <exception>
#include <jsoncons/jsoncons_config.hpp>

namespace jsoncons { namespace detail {

struct to_integer_result
{
    int64_t value;
    bool overflow;
};

// Precondition: s satisfies

// digit
// digit1-digits 
// - digit
// - digit1-digits

template <class CharT>
to_integer_result to_integer(const CharT* s, size_t length)
{
    JSONCONS_ASSERT(length > 0);

    int64_t n = 0;
    bool overflow = false;
    const CharT* end = s + length; 
    if (*s == '-')
    {
        static const int64_t min_value = (std::numeric_limits<int64_t>::min)();
        static const int64_t min_value_div_10 = min_value / 10;
        ++s;
        for (; s < end; ++s)
        {
            int64_t x = *s - '0';
            if (n < min_value_div_10)
            {
                overflow = true;
                break;
            }
            n = n * 10;
            if (n < min_value + x)
            {
                overflow = true;
                break;
            }

            n -= x;
        }
    }
    else
    {
        static const int64_t max_value = (std::numeric_limits<int64_t>::max)();
        static const int64_t max_value_div_10 = max_value / 10;
        for (; s < end; ++s)
        {
            int64_t x = *s - '0';
            if (n > max_value_div_10)
            {
                overflow = true;
                break;
            }
            n = n * 10;
            if (n > max_value - x)
            {
                overflow = true;
                break;
            }

            n += x;
        }
    }

    return to_integer_result({ n,overflow });
}

struct to_uinteger_result
{
    uint64_t value;
    bool overflow;
};

// Precondition: s satisfies

// digit
// digit1-digits 
// - digit
// - digit1-digits

template <class CharT>
to_uinteger_result to_uinteger(const CharT* s, size_t length)
{
    JSONCONS_ASSERT(length > 0);

    static const uint64_t max_value = (std::numeric_limits<uint64_t>::max)();
    static const uint64_t max_value_div_10 = max_value / 10;
    uint64_t n = 0;
    bool overflow = false;

    const CharT* end = s + length; 
    for (; s < end; ++s)
    {
        uint64_t x = *s - '0';
        if (n > max_value_div_10)
        {
            overflow = true;
            break;
        }
        n = n * 10;
        if (n > max_value - x)
        {
            overflow = true;
            break;
        }

        n += x;
    }
    return to_uinteger_result{ n,overflow };
}

#if defined(JSONCONS_HAS_MSC__STRTOD_L)

class string_to_double
{
private:
    _locale_t locale_;
public:
    string_to_double()
    {
        locale_ = _create_locale(LC_NUMERIC, "C");
    }
    ~string_to_double()
    {
        _free_locale(locale_);
    }

    char get_decimal_point() const
    {
        return '.';
    }

    double operator()(const char* s, size_t) const
    {
        const char *begin = s;
        char *end = nullptr;
        double val = _strtod_l(begin, &end, locale_);
        if (begin == end)
        {
            JSONCONS_THROW(json_exception_impl<std::invalid_argument>("Invalid float value"));
        }
        return val;
    }
private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double&) = delete;
    string_to_double& operator=(const string_to_double&) = delete;
};

#elif defined(JSONCONS_HAS_STRTOLD_L)

class string_to_double
{
private:
    locale_t locale_;
public:
    string_to_double()
    {
        locale_ = newlocale(LC_ALL_MASK, "C", (locale_t) 0);
    }
    ~string_to_double()
    {
        freelocale(locale_);
    }

    char get_decimal_point() const
    {
        return '.';
    }

    double operator()(const char* s, size_t length) const
    {
        const char *begin = s;
        char *end = nullptr;
        double val = strtold_l(begin, &end, locale_);
        if (begin == end)
        {
            JSONCONS_THROW(json_exception_impl<std::invalid_argument>("Invalid float value"));
        }
        return val;
    }

private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double& fr) = delete;
    string_to_double& operator=(const string_to_double& fr) = delete;
};

#else
class string_to_double
{
private:
    std::vector<char> buffer_;
    char decimal_point_;
public:
    string_to_double()
        : buffer_()
    {
        struct lconv * lc = localeconv();
        if (lc != nullptr && lc->decimal_point[0] != 0)
        {
            decimal_point_ = lc->decimal_point[0];    
        }
        else
        {
            decimal_point_ = '.'; 
        }
        buffer_.reserve(100);
    }

    char get_decimal_point() const
    {
        return decimal_point_;
    }

    double operator()(const char* s, size_t /*length*/) const
    {
        char *end = nullptr;
        double val = strtod(s, &end);
        if (s == end)
        {
            JSONCONS_THROW(json_exception_impl<std::invalid_argument>("string_to_double failed"));
        }
        return val;
    }

private:
    // noncopyable and nonmoveable
    string_to_double(const string_to_double& fr) = delete;
    string_to_double& operator=(const string_to_double& fr) = delete;
};
#endif

}}

#endif

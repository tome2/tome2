#pragma once

/**
 * Macro for declaring a global constexpr variable without
 * violating the ODR and without running into the SIOF.
 *
 * Shamelessly cribbed from http://stackoverflow.com/a/20374473
 */
#define PP_GLOBAL_CONSTEXPR_CONST(type, var, value)                      \
namespace global_constexpr { namespace var {                             \
template<class = void>                                                   \
struct wrapper                                                           \
{                                                                        \
     static constexpr type var = value;                                  \
};                                                                       \
template<class T>                                                        \
constexpr type wrapper<T>::var;                                          \
} }                                                                      \
namespace {                                                              \
auto const& var = global_constexpr::var::wrapper<>::var;                 \
}

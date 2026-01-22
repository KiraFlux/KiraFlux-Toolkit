// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/config.hpp"


#if kf_port_has_algorithm

#include <algorithm>


namespace kf {

using std::find;    ///< Find element in range (std implementation)
using std::for_each;///< Apply function to each element in range (std implementation)
using std::max;     ///< Return larger of two values (std implementation)
using std::min;     ///< Return smaller of two values (std implementation)

}// namespace kf

#else

namespace kf {

#ifdef min
#undef min
#endif

/// @brief Return the smaller of two values
/// @tparam T Comparable type
/// @param a First value
/// @param b Second value
/// @return Reference to the smaller value (a if equal)
template<typename T> constexpr const T &min(const T &a, const T &b) {
    return (b < a) ? b : a;
}

#ifdef max
#undef max
#endif

/// @brief Return the larger of two values
/// @tparam T Comparable type
/// @param a First value
/// @param b Second value
/// @return Reference to the larger value (a if equal)
template<typename T> constexpr const T &max(const T &a, const T &b) {
    return (a < b) ? b : a;
}

/// @brief Apply function to each element in range
/// @tparam Iterator Iterator type
/// @tparam Function Function object type
/// @param first Iterator to beginning of range
/// @param last Iterator to end of range
/// @param func Function to apply to each element
template<typename Iterator, typename Function> void for_each(Iterator first, Iterator last, Function func) {
    while (first != last) {
        func(*first);
        ++first;
    }
}

/// @brief Find first occurrence of value in range
/// @tparam Iterator Iterator type
/// @tparam T Value type to search for
/// @param first Iterator to beginning of range
/// @param last Iterator to end of range
/// @param value Value to search for
/// @return Iterator to first element equal to value, or last if not found
template<typename Iterator, typename T> Iterator find(Iterator first, Iterator last, const T &value) {
    while (first != last) {
        if (*first == value) return first;
        ++first;
    }
    return last;
}

}// namespace kf

#endif

namespace kf {

/// @brief Constrain value between lower and upper bounds
/// @tparam T Comparable type
/// @param value Value to clamp
/// @param low Lower bound (inclusive)
/// @param high Upper bound (inclusive)
/// @return Clamped value (low <= result <= high)
template<typename T> constexpr T clamp(const T &value, const T &low, const T &high) {
    return (value < low) ? low : (value > high) ? high : value;
}
}
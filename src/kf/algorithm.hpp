#pragma once

#include "kf/port/autoconfig.hpp"

#if kf_port_has_algorithm

#include <algorithm>

namespace kf {

using std::find;
using std::for_each;
using std::max;
using std::min;

}// namespace kf

#else

namespace kf {

#ifdef min
#undef min
#endif

template<typename T> constexpr const T &min(const T &a, const T &b) {
    return (b < a) ? b : a;
}

#ifdef max
#undef max
#endif

template<typename T> constexpr const T &max(const T &a, const T &b) {
    return (a < b) ? b : a;
}

template<typename T> constexpr T clamp(const T &value, const T &low, const T &high) {
    return (value < low) ? low : (value > high) ? high
                                                : value;
}

template<typename Iterator, typename Function> void for_each(Iterator first, Iterator last, Function func) {
    while (first != last) {
        func(*first);
        ++first;
    }
}

template<typename Iterator, typename T> Iterator find(Iterator first, Iterator last, const T &value) {
    while (first != last) {
        if (*first == value) return first;
        ++first;
    }
    return last;
}

}// namespace kf

#endif
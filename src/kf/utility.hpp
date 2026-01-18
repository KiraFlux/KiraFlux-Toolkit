#pragma once

#include "kf/port/autoconfig.hpp"

#if kf_port_has_utility

#include <utility>

namespace kf {

using std::forward;
using std::make_pair;
using std::move;
using std::pair;
using std::swap;

}// namespace kf

#else

#include "kf/type_traits.hpp"

namespace kf {

// Simplified move implementation
template<typename T> constexpr typename kf::remove_reference<T>::type &&move(T &&arg) noexcept {
    return static_cast<typename kf::remove_reference<T>::type &&>(arg);
}

// Simplified forward implementation
template<typename T> constexpr T &&forward(typename kf::remove_reference<T>::type &arg) noexcept {
    return static_cast<T &&>(arg);
}

template<typename T> constexpr T &&forward(typename kf::remove_reference<T>::type &&arg) noexcept {
    return static_cast<T &&>(arg);
}

// Basic swap
template<typename T> void swap(T &a, T &b) {
    T temp = move(a);
    a = move(b);
    b = move(temp);
}

// Basic pair implementation
template<typename T1, typename T2> struct pair {
    T1 first;
    T2 second;

    pair() = default;
    pair(const T1 &f, const T2 &s) :
        first{f}, second{s} {}

    template<typename U1, typename U2> pair(U1 &&f, U2 &&s) :
        first(forward<U1>(f)), second(forward<U2>(s)) {}
};

template<typename T1, typename T2> pair<T1, T2> make_pair(T1 &&f, T2 &&s) {
    return pair<T1, T2>(forward<T1>(f), forward<T2>(s));
}

}// namespace kf

#endif
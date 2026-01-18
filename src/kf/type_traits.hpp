#pragma once

#include "kf/port/autoconfig.hpp"

#if kf_port_has_type_traits
#include <type_traits>

namespace kf {

using std::conditional;
using std::enable_if;
using std::false_type;
using std::integral_constant;
using std::is_arithmetic;
using std::is_base_of;
using std::is_const;
using std::is_convertible;
using std::is_floating_point;
using std::is_integral;
using std::is_null_pointer;
using std::is_pointer;
using std::is_reference;
using std::is_same;
using std::is_void;
using std::is_volatile;
using std::remove_const;
using std::remove_cv;
using std::remove_reference;
using std::remove_volatile;
using std::true_type;

}// namespace kf

#else

namespace kf {
// Basic type traits implementation for AVR

template<typename T, T v> struct integral_constant {
    static constexpr T value = v;// NOLINT(*-dynamic-static-initializers)
    using value_type = T;
    using type = integral_constant;
    constexpr operator value_type() const noexcept { return value; }// NOLINT(*-explicit-constructor)
};

using true_type = integral_constant<bool, true>;
using false_type = integral_constant<bool, false>;

// is_same
template<typename T, typename U> struct is_same : false_type {};

template<typename T> struct is_same<T, T> : true_type {};

// remove_reference
template<typename T> struct remove_reference {
    using type = T;
};

template<typename T> struct remove_reference<T &> {
    using type = T;
};

template<typename T> struct remove_reference<T &&> {
    using type = T;
};

// remove_const

template<typename T> struct remove_const {
    using type = T;
};

template<typename T> struct remove_const<const T> {
    using type = T;
};

// remove_volatile

template<typename T> struct remove_volatile {
    using type = T;
};

template<typename T> struct remove_volatile<volatile T> {
    using type = T;
};

// remove_cv
template<typename T> struct remove_cv {
    using type = typename remove_volatile<typename remove_const<T>::type>::type;
};

// enable_if
template<bool, typename T = void> struct enable_if {};

template<typename T> struct enable_if<true, T> {
    using type = T;
};

// is_integral
template<typename T> struct is_integral : false_type {};
template<> struct is_integral<bool> : true_type {};
template<> struct is_integral<char> : true_type {};
template<> struct is_integral<signed char> : true_type {};
template<> struct is_integral<unsigned char> : true_type {};
template<> struct is_integral<short> : true_type {};
template<> struct is_integral<unsigned short> : true_type {};
template<> struct is_integral<int> : true_type {};
template<> struct is_integral<unsigned int> : true_type {};
template<> struct is_integral<long> : true_type {};
template<> struct is_integral<unsigned long> : true_type {};
template<> struct is_integral<long long> : true_type {};
template<> struct is_integral<unsigned long long> : true_type {};

// is_floating_point
template<typename T> struct is_floating_point : false_type {};
template<> struct is_floating_point<float> : true_type {};
template<> struct is_floating_point<double> : true_type {};
template<> struct is_floating_point<long double> : true_type {};

// is_arithmetic
template<typename T> struct is_arithmetic : integral_constant<bool, is_integral<T>::value || is_floating_point<T>::value> {};

// conditional
template<bool B, typename T, typename F> struct conditional {
    using type = T;
};

template<typename T, typename F> struct conditional<false, T, F> {
    using type = F;
};

// Note: is_base_of is complex to implement without compiler support
// For AVR, we'll use a simplified version that may not cover all cases
template<typename Base, typename Derived> struct is_base_of {
private:
    template<typename T> static true_type test(Base *);

    template<typename> static false_type test(...);

public:
    static constexpr bool value = decltype(test<Derived>(static_cast<Derived *>(0)))::value;// NOLINT(*-dynamic-static-initializers)
};

}// namespace kf

#endif
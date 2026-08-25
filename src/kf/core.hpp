// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file core.hpp Toolkit's common components

#pragma once

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace kf {

/// @brief Tag type for constructing an empty Option (None).
/// @note Use the global constant kf::none to create empty options.
struct NoneType final {
    explicit NoneType() = default;
};

/// @brief Global constant for constructing an empty Option (None).
constexpr NoneType none{};

/// @brief Type aliases for standard integer and floating‑point types.
inline namespace primitives {

using u8 = uint8_t;
using i8 = int8_t;

using u16 = uint16_t;
using i16 = int16_t;

using u32 = uint32_t;
using i32 = int32_t;

using u64 = uint64_t;
using i64 = int64_t;

using f32 = float;
using f64 = double;

using usize = size_t;
using isize = ptrdiff_t;

}// namespace primitives

/// @brief Compile‑time type utilities.
inline namespace type_traits {

template<typename F, typename Signature> struct is_callable : std::false_type {};

template<typename F, typename R, typename... Args> struct is_callable<F, R(Args...)> : std::is_invocable_r<R, F, Args...> {};

template<usize bytes> struct integer_by_bytes {
    static_assert(bytes <= 8);
};

template<> struct integer_by_bytes<1> {
    using min_signed_type = i8;
    using min_unsigned_type = u8;
};

template<> struct integer_by_bytes<2> {
    using min_signed_type = i16;
    using min_unsigned_type = u16;
};

template<> struct integer_by_bytes<3> {
    using min_signed_type = i32;
    using min_unsigned_type = u32;
};

template<> struct integer_by_bytes<4> : integer_by_bytes<3> {};

template<> struct integer_by_bytes<5> {
    using min_signed_type = i64;
    using min_unsigned_type = u64;
};

template<> struct integer_by_bytes<6> : integer_by_bytes<5> {};

template<> struct integer_by_bytes<7> : integer_by_bytes<5> {};

template<> struct integer_by_bytes<8> : integer_by_bytes<5> {};

template<usize bits> using integer_by_bits = integer_by_bytes<(bits + 7) / 8>;

// aliases

template<usize bytes> using signed_integer_by_bytes_t = integer_by_bytes<bytes>::min_signed_type;

template<usize bytes> using unsigned_integer_by_bytes_t = integer_by_bytes<bytes>::min_unsigned_type;

template<usize bits> using signed_integer_by_bits_t = integer_by_bits<bits>::min_signed_type;

template<usize bits> using unsigned_integer_by_bits_t = integer_by_bits<bits>::min_unsigned_type;

}// namespace type_traits

inline namespace concepts {

template<typename Impl, typename Tag>
concept implements = std::derived_from<Impl, Tag>;

template<typename T>
concept arithmetic = std::is_arithmetic_v<T>;

template<typename T>
concept trivial = std::is_trivially_copyable_v<T> and std::is_trivially_destructible_v<T>;

template<typename T>
concept c_string = (

    std::is_same_v<T, char *> or
    std::is_same_v<T, char const *> or
    (std::is_array_v<T> and std::is_same_v<std::remove_extent_t<T>, char>)

);

/// @brief Concept for callable objects with a specific signature.
/// @tparam F    The callable type.
/// @tparam Signature Function type.
template<typename F, typename Signature>
concept callable = is_callable<F, Signature>::value;

template<typename T, typename U>
concept type_like = std::is_same_v<std::remove_cv_t<T>, U>;

template<typename T>
concept enum_type = std::is_enum_v<T>;

template<typename T>
concept integer_type = std::is_integral_v<T>;

template<typename T>
concept float_type = std::is_floating_point_v<T>;

}// namespace concepts

}// namespace kf
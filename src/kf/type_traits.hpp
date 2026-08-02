// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    type_traits.hpp
/// @brief   Compile‑time type utilities.

#pragma once

#include "kf/primitives.hpp"

namespace kf {

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

}// namespace kf
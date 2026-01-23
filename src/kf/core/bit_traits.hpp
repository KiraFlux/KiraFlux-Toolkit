// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <kf/core/aliases.hpp>

namespace kf {

template<u8 bytes> struct byte_traits;

template<> struct byte_traits<1> {
    using min_signed = i8;
    using min_unsigned = u8;
};

template<> struct byte_traits<2> {
    using min_signed = i16;
    using min_unsigned = u16;
};

template<> struct byte_traits<3> {
    using min_signed = i32;
    using min_unsigned = u32;
};

template<> struct byte_traits<4> : byte_traits<3> {};

template<> struct byte_traits<5> {
    using min_signed = i64;
    using min_unsigned = u64;
};

template<> struct byte_traits<6> : byte_traits<5> {};

template<> struct byte_traits<7> : byte_traits<5> {};

template<> struct byte_traits<8> : byte_traits<5> {};

template<u8 bits> struct bit_traits : byte_traits<(bits + 7) / 8> {};

}// namespace kf

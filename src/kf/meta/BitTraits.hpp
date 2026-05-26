// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/primitives.hpp"

namespace kf::meta {

template<u8 bytes> struct ByteTraits;

template<> struct ByteTraits<1> {
    using min_signed = i8;
    using min_unsigned = u8;
};

template<> struct ByteTraits<2> {
    using min_signed = i16;
    using min_unsigned = u16;
};

template<> struct ByteTraits<3> {
    using min_signed = i32;
    using min_unsigned = u32;
};

template<> struct ByteTraits<4> : ByteTraits<3> {};

template<> struct ByteTraits<5> {
    using min_signed = i64;
    using min_unsigned = u64;
};

template<> struct ByteTraits<6> : ByteTraits<5> {};

template<> struct ByteTraits<7> : ByteTraits<5> {};

template<> struct ByteTraits<8> : ByteTraits<5> {};

template<u8 bits> struct BitTraits : ByteTraits<(bits + 7) / 8> {};

}// namespace kf::meta

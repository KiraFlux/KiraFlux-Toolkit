// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    MacAddress.hpp

#pragma once

#include "kf/Array.hpp"
#include "kf/core.hpp"

namespace kf::internal {

using MacAddressBase = Array<u8, 6>;

}// namespace kf::internal

namespace kf {

/// @brief Represents a 6‑byte MAC address
/// @note Trivially copyable, safe for serialization
struct MacAddress :

    internal::MacAddressBase

{
    constexpr MacAddress() noexcept :
        internal::MacAddressBase{} {}

    constexpr MacAddress(u8 a, u8 b, u8 c, u8 d, u8 e, u8 f) noexcept :
        internal::MacAddressBase{.items = {a, b, c, d, e, f}} {}
};

}// namespace kf
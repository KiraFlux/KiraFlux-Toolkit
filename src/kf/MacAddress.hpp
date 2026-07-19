// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Array.hpp"
#include "kf/String.hpp"
#include "kf/mixin/Representable.hpp"
#include "kf/primitives.hpp"

namespace kf::internal {

using MacAddressBase = Array<u8, 6>;

using MacAddressStringType = Array<char, 14u>;

}// namespace kf::internal

namespace kf {

/// @brief MAC address (6 bytes)
/// @note Trivially copyable, safe for serialization
/// @note Provides conversion to human‑readable string "aabb-ccdd-eeff"
struct MacAddress :

    internal::MacAddressBase,
    mixin::Representable<MacAddress, internal::MacAddressStringType>

{
    constexpr MacAddress() noexcept :
        internal::MacAddressBase{} {}

    constexpr MacAddress(u8 a, u8 b, u8 c, u8 d, u8 e, u8 f) noexcept :
        internal::MacAddressBase{.items = {a, b, c, d, e, f}} {}

private:
    KF_IMPL_REPRESENTABLE(MacAddress, internal::MacAddressStringType);
    auto reprImpl() const noexcept {
        const auto bytes = this->data();
        return String::formatted<internal::MacAddressStringType::items_total>(

            "%02x%02x-%02x%02x-%02x%02x",// FIXME: formatting with hex base
            bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]

        );
    }
};

}// namespace kf
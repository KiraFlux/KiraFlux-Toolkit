// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/memory/Array.hpp"
#include "kf/memory/StaticString.hpp"
#include "kf/mixin/StringRepresentable.hpp"
#include "kf/primitives.hpp"

namespace kf::internal {

using MacAddressStringType = memory::StaticString<14>;

}// namespace kf::internal

namespace kf::network {

/// @brief MAC address (6 bytes)
/// @note Trivially copyable, safe for serialization
/// @note Provides conversion to human‑readable string "aabb-ccdd-eeff"
struct MacAddress final : memory::Array<u8, 6>, mixin::StringRepresentable<MacAddress, internal::MacAddressStringType> {
private:
    KF_IMPL_STRING_REPRESENTABLE(MacAddress, internal::MacAddressStringType);
    auto toStringImpl() const noexcept {
        const auto bytes = this->data();
        return internal::MacAddressStringType::formatted(
            "%02x%02x-%02x%02x-%02x%02x",
            bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]);
    }
};

}// namespace kf::network
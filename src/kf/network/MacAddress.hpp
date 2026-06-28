// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Array.hpp"
#include "kf/memory/StaticString.hpp"
#include "kf/mixin/StringRepresentable.hpp"
#include "kf/primitives.hpp"

namespace kf::internal {

using MacAddressBase = Array<u8, 6>;

using MacAddressStringType = memory::StaticString<14>;

}// namespace kf::internal

namespace kf::network {

/// @brief MAC address (6 bytes)
/// @note Trivially copyable, safe for serialization
/// @note Provides conversion to human‑readable string "aabb-ccdd-eeff"
struct MacAddress final : internal::MacAddressBase, mixin::StringRepresentable<MacAddress, internal::MacAddressStringType> {

    using internal::MacAddressBase::MacAddressBase;

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
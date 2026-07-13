// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/MacAddress.hpp"

namespace kf::mixin {

/// @brief Base class for objects that own a MAC address
/// @note Provides read‑only access to the address
struct MacAddressed {
    explicit constexpr MacAddressed(const MacAddress &mac_address) noexcept :
        _mac_address{mac_address} {}

    /// @brief Get MAC address
    [[nodiscard]] constexpr const MacAddress &mac() const noexcept {
        return _mac_address;
    }

protected:
    MacAddress _mac_address;
};

}// namespace kf::mixin
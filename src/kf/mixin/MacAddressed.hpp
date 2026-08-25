// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/MacAddressed.hpp
/// @brief   Adds a MAC address member with read‑only access.

#pragma once

#include "kf/MacAddress.hpp"

namespace kf::mixin {

/// @brief Base class for objects that own a MAC address
/// @note Provides read‑only access to the address
struct MacAddressed {
    explicit constexpr MacAddressed(MacAddress const &mac_address) noexcept :
        _mac_address{mac_address} {}

    /// @brief Get MAC address
    [[nodiscard]] constexpr MacAddress const &mac() const noexcept {
        return _mac_address;
    }

protected:
    MacAddress _mac_address;
};

}// namespace kf::mixin
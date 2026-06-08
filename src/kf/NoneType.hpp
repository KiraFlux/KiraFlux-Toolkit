// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf {

/// @brief Tag type for constructing an empty Option (None)
/// @note Use the global constant kf::none to create empty options.
struct NoneType final {
    explicit NoneType() = default;
};

/// @brief Global constant for constructing an empty Option (None).
constexpr NoneType none{};

}// namespace kf
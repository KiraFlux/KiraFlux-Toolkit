// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

struct ResettableTag {};

/// @brief CRTP mixin reset.
/// @tparam Impl Derived class (must provide `resetImpl()`).
template<typename Impl> struct Resettable : ResettableTag {

    /// @brief Reset: full state-reset
    void reset() noexcept { static_cast<Impl *>(this)->resetImpl(); }
};

}// namespace kf::mixin

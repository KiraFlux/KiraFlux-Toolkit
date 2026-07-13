// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

struct ResettableTag {};

/// @brief CRTP mixin reset.
/// @tparam Impl The derived class that must implement `resetImpl()`.
template<typename Impl> struct Resettable : ResettableTag {

    /// @brief Reset: full state-reset
    constexpr void reset() noexcept {
        static_cast<Impl *>(this)->resetImpl();
    }
};

}// namespace kf::mixin

#define KF_IMPL_RESETTABLE(...) friend struct ::kf::mixin::Resettable<__VA_ARGS__>

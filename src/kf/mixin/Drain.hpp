// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/Drain.hpp

#pragma once

namespace kf::mixin {

struct DrainTag {};

/// @brief Adds `drain()` to return output consume.
/// @tparam Impl Drain implementation class with `T drainImpl() noexcept` method
/// @tparam T   The type of the drained value.
template<typename Impl, typename T> struct Drain : DrainTag {

    /// @brief Drain the contained data (move out) and reset the source.
    /// @return The drained value (moved out).
    [[nodiscard]] T drain() noexcept {
        return static_cast<Impl *>(this)->drainImpl();
    }
};

}// namespace kf::mixin

#define KF_IMPL_DRAIN(...) friend struct ::kf::mixin::Drain<__VA_ARGS__>

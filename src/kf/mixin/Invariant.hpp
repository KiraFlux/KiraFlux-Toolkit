// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

/// @brief CRTP mixin that provides state checking (Some/None) for optional-like types
/// @tparam Impl The derived class (must implement `bool isSomeImpl() const noexcept`).
/// @note Adds `isSome()` and `isNone()` to the derived class.
template<typename Impl> struct Invariant {

    /// @brief Checks whether the object contains a Some
    /// @return `true` if a value is present, `false` otherwise.
    [[nodiscard]] constexpr bool isSome() const noexcept {
        return static_cast<const Impl *>(this)->isSomeImpl();
    }

    /// @brief Checks whether the object is None
    /// @return `true` if empty, `false` otherwise.
    [[nodiscard]] constexpr bool isNone() const noexcept {
        return not isSome();
    }
};

}// namespace kf::mixin

#define KF_IMPL_INVARIANT(...) friend struct ::kf::mixin::Invariant<__VA_ARGS__>;
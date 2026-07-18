// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

/// @brief Adds `==` and `!=`
/// @tparam Impl Equatable Implementation class with `constexpr bool isEqualsImpl(const Impl &) const noexcept`
template<typename Impl> struct Equatable {

    [[nodiscard]] constexpr bool operator==(const Impl &other) const noexcept {
        return static_cast<const Impl *>(this)->isEqualsImpl(other);
    }

    [[nodiscard]] constexpr bool operator!=(const Impl &other) const noexcept {
        return not(*this == other);
    }
};

}// namespace kf::mixin

#define KF_IMPL_EQUATABLE(...) friend struct ::kf::mixin::Equatable<__VA_ARGS__>

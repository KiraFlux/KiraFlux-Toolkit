// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

/// @brief Adds `==` and `!=`
/// @tparam Impl Equatable Implementation class with `constexpr bool IsEqualsImpl(const Impl &) const noexcept`
template<typename Impl> struct Equatable {

    [[nodiscard]] constexpr bool operator==(const Impl &other) const noexcept {
        return static_cast<const Impl *>(this)->IsEqualsImpl(other);
    }

    [[nodiscard]] constexpr bool operator!=(const Impl &other) const noexcept {
        return not(*this == other);
    }
};

}// namespace kf::mixin

#define KF_IMPL_EQUATABLE(__impl__) friend struct ::kf::mixin::Equatable<__impl__>

// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

struct LengthTag {};

/// @brief Adds `length()`
/// @tparam Impl Length implementation class with `constexpr T lengthImpl() const noexcept` method
/// @tparam T length value type
template<typename Impl, typename T> struct Length : LengthTag {

    /// @brief Get Length
    [[nodiscard]] constexpr T length() const noexcept {
        static_cast<Impl *>(this)->lengthImpl();
    }
};

}// namespace kf::mixin

#define KF_IMPL_LENGTH(__impl__, __type__) friend struct ::kf::mixin::Length<__impl__, __type__>
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/Length.hpp
/// @brief   Adds length() method to a class via CRTP.

#pragma once

namespace kf::mixin {

struct LengthTag {};

/// @brief Adds `length()`
/// @tparam Impl Length implementation class with `constexpr T lengthImpl() const noexcept` method
/// @tparam T length value type
template<typename Impl, typename T> struct Length : LengthTag {

    /// @brief Get Length
    [[nodiscard]] constexpr T length() const noexcept {
        return static_cast<Impl const *>(this)->lengthImpl();
    }
};

}// namespace kf::mixin

#define KF_IMPL_LENGTH(...) friend struct ::kf::mixin::Length<__VA_ARGS__>

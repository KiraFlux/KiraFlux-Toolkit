// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/Readable.hpp
/// @brief   Adds read() -> Option<T> for consuming items.

#pragma once

#include <utility>

#include "kf/Option.hpp"

namespace kf::mixin {

struct ReadableTag {};

/// @brief Add `read() -> Option<T>`
/// @tparam Impl Readable mixin implementation class
/// @tparam T Value type
template<typename Impl, typename T> struct Readable : ReadableTag {

    /// @brief Read Value
    /// @return Option with moved value if read success, `none` otherwise
    [[nodiscard]] constexpr auto read() -> Option<T> {
        return static_cast<Impl *>(this)->readImpl();
    }
};

}// namespace kf::mixin

#define KF_IMPL_READABLE(...) friend struct ::kf::mixin::Readable<__VA_ARGS__>

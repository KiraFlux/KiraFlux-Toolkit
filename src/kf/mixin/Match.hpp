// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/Match.hpp

#pragma once

namespace kf::mixin {

struct MatchTag {};

/// @brief Adds `match(T)` to check if the object matches a given value.
/// @tparam Impl Match implementation class with `bool matchImpl(T value) noexcept` method
/// @tparam T   The type of the matched value.
template<typename Impl, typename T> struct Match : MatchTag {

    /// @brief Compare the object against the provided value.
    /// @param value The value to match against.
    /// @return `true` if the object matches, `false` otherwise.
    [[nodiscard]] bool match(T value) noexcept {
        return static_cast<Impl *>(this)->matchImpl(value);
    }
};

}// namespace kf::mixin

#define KF_IMPL_MATCH(...) friend struct ::kf::mixin::Match<__VA_ARGS__>

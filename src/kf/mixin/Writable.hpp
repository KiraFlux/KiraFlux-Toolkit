// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

namespace kf::mixin {

struct WritableTag {};

}// namespace kf::mixin

namespace kf::internal {

template<typename Impl, typename T> struct WritableBase : mixin::WritableTag {

    /// @brief Write Value
    /// @param value Value to write (perfect-forwarded)
    /// @return `true` if write successful, `false` otherwise
    [[nodiscard]] constexpr bool write(auto &&value) noexcept {
        return static_cast<Impl *>(this)->writeImpl(std::forward<decltype(value)>(value));
    }
};

}// namespace kf::internal

namespace kf::mixin {

/// @brief Add `write(auto &&) -> bool`
/// @tparam Impl Writable mixin implementation class
/// @tparam T Value type
template<typename Impl, typename T> struct Writable;

template<typename Impl, typename T> struct Writable : internal::WritableBase<Impl, T> {};

// TODO: char specialization

}// namespace kf::mixin

#define KF_IMPL_WRITABLE(...) friend struct ::kf::internal::WritableBase<__VA_ARGS__>

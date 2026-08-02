// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/ReadAvailable.hpp
/// @brief   Adds availableForRead() to query available data.

#pragma once

#include "kf/primitives.hpp"

namespace kf::mixin {

struct ReadAvailableTag {};

/// @brief Adds `availableForRead()` to query how many bytes/items can be read without blocking.
/// @tparam Impl Implementation class with `usize availableForReadImpl() const noexcept` method.
/// @note The value may change between calls; intended for non‑blocking polling.
template<typename Impl> struct ReadAvailable : ReadAvailableTag {

    /// @brief Get the number of bytes/items currently available to read.
    [[nodiscard]] usize availableForRead() const noexcept {
        return static_cast<Impl const *>(this)->availableForReadImpl();
    }
};

}// namespace kf::mixin

#define KF_IMPL_READ_AVAILABLE(...) friend struct ::kf::mixin::ReadAvailable<__VA_ARGS__>

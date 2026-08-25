// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/WriteAvailable.hpp
/// @brief   Adds availableForWrite() to query free space.

#pragma once

#include "kf/core.hpp"

namespace kf::mixin {

struct WriteAvailableTag {};

/// @brief Adds `availableForWrite()` to query how many bytes/items can be written without blocking.
/// @tparam Impl Implementation class with `usize availableForWriteImpl() const noexcept` method.
/// @note The value is advisory; actual write may still block if buffer is shared.
template<typename Impl> struct WriteAvailable : WriteAvailableTag {

    /// @brief Get the amount of free space available for writing.
    [[nodiscard]] usize availableForWrite() const noexcept {
        return static_cast<Impl const *>(this)->availableForWriteImpl();
    }
};

}// namespace kf::mixin

#define KF_IMPL_WRITE_AVAILABLE(...) friend struct ::kf::mixin::WriteAvailable<__VA_ARGS__>

// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

struct NonCopyable {

    /// @brief Deleted copy constructor (non-copyable)
    NonCopyable(NonCopyable const &) = delete;

    /// @brief Deleted copy assignment operator (non-copyable)
    NonCopyable &operator=(NonCopyable const &) = delete;

    // Movable

    NonCopyable(NonCopyable &&) = default;
    NonCopyable &operator=(NonCopyable &&) = default;

protected:
    /// @brief Protected default constructor (ensures single instance)
    NonCopyable() = default;
};

}// namespace kf::mixin
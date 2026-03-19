// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

struct NonCopyable {

    /// @brief Deleted copy constructor (non-copyable)
    NonCopyable(const NonCopyable &) = delete;

    /// @brief Deleted copy assignment operator (non-copyable)
    NonCopyable &operator=(const NonCopyable &) = delete;

protected:
    /// @brief Protected default constructor (ensures single instance)
    NonCopyable() = default;

    /// @brief Protected destructor (ensures controlled lifetime)
    ~NonCopyable() = default;
};

}// namespace kf::mixin

#define KF_IMPL_NON_COPYABLE() friend struct kf::mixin::NonCopyable
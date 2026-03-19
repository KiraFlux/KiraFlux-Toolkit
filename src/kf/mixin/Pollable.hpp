// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

struct PollableTag {};

/// @brief CRTP mixin poll (Periodic polling)
/// @tparam Impl Derived class (must provide `pollImpl()`).
template<typename Impl> struct Pollable : PollableTag {

    /// @brief Poll
    void poll() noexcept { static_cast<Impl *>(this)->pollImpl(); }
};

}// namespace kf::mixin

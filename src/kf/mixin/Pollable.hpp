// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

struct PollableTag {};

/// @brief CRTP mixin poll (Periodic polling)
/// @tparam Impl The derived class that must implement `pollImpl()`.
template<typename Impl> struct Pollable : PollableTag {

    /// @brief Performs periodic polling.
    void poll() noexcept {
        static_cast<Impl *>(this)->pollImpl();
    }
};

}// namespace kf::mixin

#define KF_IMPL_POLLABLE(...) friend struct ::kf::mixin::Pollable<__VA_ARGS__>

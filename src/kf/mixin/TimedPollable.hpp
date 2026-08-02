// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/TimedPollable.hpp
/// @brief   Adds poll(now) with a timestamp argument.

#pragma once

#include "kf/units.hpp"

namespace kf::mixin {

struct TimedPollableTag {};

/// @brief CRTP mixin poll (Periodic polling)
/// @tparam Impl The derived class that must implement `pollImpl(units::Milliseconds)`.
template<typename Impl> struct TimedPollable : TimedPollableTag {

    /// @brief Performs time‑aware polling.
    /// @param now Current timestamp (milliseconds).
    void poll(units::Milliseconds now) noexcept {
        static_cast<Impl *>(this)->pollImpl(now);
    }
};

}// namespace kf::mixin

#define KF_IMPL_TIMED_POLLABLE(...) friend struct ::kf::mixin::TimedPollable<__VA_ARGS__>

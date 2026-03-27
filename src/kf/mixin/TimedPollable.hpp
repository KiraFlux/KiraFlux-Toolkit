// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/math/units.hpp"

namespace kf::mixin {

struct TimedPollableTag {};

/// @brief CRTP mixin poll (Periodic polling)
/// @tparam Impl The derived class that must implement `pollImpl(kf::math::Milliseconds)`.
template<typename Impl> struct TimedPollable : TimedPollableTag {

    /// @brief Performs time‑aware polling.
    /// @param now Current timestamp (milliseconds).
    void poll(kf::math::Milliseconds now) noexcept { static_cast<Impl *>(this)->pollImpl(now); }
};

}// namespace kf::mixin

#define KF_IMPL_TIMED_POLLABLE(__impl__) friend struct ::kf::mixin::TimedPollable<__impl__>

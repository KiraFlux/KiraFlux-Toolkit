// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/math/units.hpp"

namespace kf::mixin {

struct TimedPollableTag {};

/// @brief CRTP mixin poll (Periodic polling)
/// @tparam Impl Derived class (must provide `pollImpl(kf::math::Milliseconds now)`).
template<typename Impl> struct TimedPollable : TimedPollableTag {

    /// @brief Poll
    /// @note Must be called regularly (e.g., in main loop)
    void poll(kf::math::Milliseconds now) noexcept { static_cast<Impl *>(this)->pollImpl(now); }
};

}// namespace kf::mixin

#define KF_IMPL_TIMED_POLLABLE(__impl__) friend struct ::kf::mixin::TimedPollable<__impl__>

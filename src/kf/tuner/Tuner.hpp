// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Pollable.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf::tuner {

struct TunerTag {};

/// @brief CRTP base class for all tuners.
/// @tparam Impl Derived class implementing the actual tuning logic.
/// @note Derived classes must provide:
///
///       - `bool runningImpl() const noexcept`
///
///       - `void resetImpl() noexcept` (from mixin::Resettable)
///
///       - `void pollImpl() noexcept` (from mixin::Pollable)
template<typename Impl> struct Tuner : TunerTag, mixin::Resettable<Impl>, mixin::Pollable<Impl> {

    /// @brief Check if the tuner is still running (collecting or calculating).
    [[nodiscard]] bool running() const noexcept { return static_cast<const Impl *>(this)->runningImpl(); }
};

}// namespace kf::tuner

#define KF_IMPL_TUNER(__impl__) friend struct kf::tuner::Tuner<__impl__>
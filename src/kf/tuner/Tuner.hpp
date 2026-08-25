// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Poll.hpp"
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
///       - `void pollImpl(kf::units::Milliseconds now) noexcept` (from mixin::Poll)
template<typename Impl> struct Tuner :

    TunerTag,
    mixin::NonCopyable,
    mixin::Resettable<Impl>,
    mixin::Poll<Impl>

{

    /// @brief Check if the tuner is still running (collecting or calculating).
    [[nodiscard]] bool running() const noexcept {
        return static_cast<Impl const *>(this)->runningImpl();
    }
};

}// namespace kf::tuner

#define KF_IMPL_TUNER(...)                       \
    friend struct kf::tuner::Tuner<__VA_ARGS__>; \
    KF_IMPL_RESETTABLE(__VA_ARGS__);             \
    KF_IMPL_POLL(__VA_ARGS__)

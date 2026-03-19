// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Resettable.hpp"

namespace kf::tuner {

struct TunerTag {};

/// @brief CRTP base class for all tuners.
/// @tparam Impl Derived class implementing the actual tuning logic.
/// @note Derived classes must provide:
///
///       - `void startImpl() noexcept`
///
///       - `bool runningImpl() const noexcept`
///
///       - `void pollImpl() noexcept`
template<typename Impl> struct Tuner : TunerTag, mixin::Resettable<Impl> {

    /// @brief Check if the tuner is still running (collecting or calculating).
    [[nodiscard]] bool running() const noexcept { return impl().runningImpl(); }

    /// @brief Process one iteration (collect or calculate). Must be called repeatedly while running.
    void poll() noexcept { impl().pollImpl(); }

private:
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::tuner
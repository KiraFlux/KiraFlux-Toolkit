// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"

#include "kf/tuner/Tuner.hpp"

namespace kf::tuner {

/// @brief CRTP base class for tuners that collect a fixed number of samples before calculating.
/// @tparam Impl Derived class implementing the actual tuning logic.
/// @tparam I    Configuration type that will be updated after calculation.
/// @note The derived class must provide:
///
///       - `void startImpl() noexcept`   – called when collection begins.
///
///       - `void pollImpl() noexcept`    – called for each sample; should process the current sample (e.g., update min/max/sum).
///
///       - `void calculateImpl(I&) noexcept` – called after all samples are collected; updates the configuration.
///
/// @details This class manages a state machine (Idle → Processing → Calculating → Idle).
///          After `start()`, the caller must repeatedly call `poll()` until `running()` returns false.
///          The `poll()` implementation of this class will invoke the derived class's `pollImpl()`
///          for each sample and, once the required number of samples is reached, transition to
///          the Calculating state and call `calculateImpl()`.
template<typename Impl, typename I> struct SampleCollectingTuner : Tuner<SampleCollectingTuner<Impl, I>> {
    using ConfigImpl = I;

    /// @brief Total number of samples to collect before calculating.
    const u16 samples_total;

    explicit SampleCollectingTuner(ConfigImpl &config, u16 samples) noexcept :
        _config{config}, samples_total{samples} {}

private:
    ConfigImpl &_config;
    u16 _samples_processed{0};

    enum class State : u8 {
        Idle,
        Processing,
        Calculating,
    } _state{State::Idle};

    // impl

    friend struct kf::mixin::Resettable<SampleCollectingTuner<Impl, I>>;

    void resetImpl() noexcept {
        _state = State::Processing;
        _samples_processed = 0;
        impl().resetImpl();
    }

    friend struct kf::tuner::Tuner<SampleCollectingTuner<Impl, I>>;

    [[nodiscard]] bool runningImpl() const noexcept { return _state != State::Idle; }

    void pollImpl() noexcept {
        switch (_state) {
            case State::Idle://
                return;

            case State::Processing: {
                impl().pollImpl();
                _samples_processed += 1;

                if (_samples_processed >= samples_total) {
                    _state = State::Calculating;
                }
            }
                return;

            case State::Calculating: {
                impl().calculateImpl(_config);
                _state = State::Idle;
            }
                return;
        }
    }

    // CRTP
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::tuner
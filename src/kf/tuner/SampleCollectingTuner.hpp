// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"

#include "kf/tuner/Tuner.hpp"

namespace kf::tuner {

/// @brief CRTP base class for tuners that collect a fixed number of samples before calculating.
/// @tparam Impl Derived class implementing the actual tuning logic.
/// @tparam T    Configuration type that will be updated after calculation.
/// @note The derived class must provide:
///
///       - `void resetImpl() noexcept`   – called when collection begins.
///
///       - `void pollImpl() noexcept`    – called for each sample; should process the current sample (e.g., update min/max/sum).
///
///       - `void calculateImpl(T&) noexcept` – called after all samples are collected; updates the configuration.
///
/// @details This class manages a state machine (Idle -> Running -> Calculating -> Idle).
///          After `reset()`, the caller must repeatedly call `poll()` until `running()` returns false.
///          The `poll()` implementation of this class will invoke the derived class's `pollImpl()`
///          for each sample and, once the required number of samples is reached, transition to
///          the Calculating state and call `calculateImpl()`.
template<typename Impl, typename T> struct SampleCollectingTuner : Tuner<SampleCollectingTuner<Impl, T>> {
    using ConfigType = T;
    using SampleCounterType = u16;

    enum class State : u8 {
        Idle,
        Running,
        Calculating,
    };

    /// @brief Total number of samples to collect before calculating.
    const SampleCounterType samples_total;

    explicit SampleCollectingTuner(ConfigType &config, SampleCounterType samples) noexcept :
        _config{config}, samples_total{samples} {}

private:
    ConfigType &_config;
    SampleCounterType _samples_processed{0};
    State _state{State::Idle};

    // impl

    using This = SampleCollectingTuner<Impl, T>;

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept {
        _state = State::Running;
        _samples_processed = 0;
        impl().resetImpl();
    }

    KF_IMPL_POLLABLE(This);
    void pollImpl() noexcept {
        switch (_state) {
            case State::Idle://
                return;

            case State::Running: {
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

    friend struct kf::tuner::Tuner<This>;
    [[nodiscard]] bool runningImpl() const noexcept { return _state != State::Idle; }

    // CRTP
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::tuner
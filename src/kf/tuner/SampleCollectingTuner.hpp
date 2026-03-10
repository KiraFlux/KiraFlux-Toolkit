// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"

#include "kf/tuner/Tuner.hpp"

namespace kf::tuner {

template<typename Impl, typename I> struct SampleCollectingTuner : Tuner<SampleCollectingTuner<Impl, I>> {
    using ConfigImpl = I;

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

    //

    // Tuner impl
    friend struct kf::tuner::Tuner<SampleCollectingTuner<Impl, I>>;

    [[nodiscard]] bool runningImpl() const noexcept { return _state != State::Idle; }

    void startImpl() noexcept {
        _state = State::Processing;
        _samples_processed = 0;
        impl().startImpl();
    }

    void pollImpl() noexcept {
        switch (_state) {
            case State::Idle://
                return;

            case State::Processing: {
                impl().pollImpl();
                _samples_processed += 1;

                const bool is_ready = _samples_processed >= samples_total;

                if (is_ready) {
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
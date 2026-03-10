// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"

namespace kf::tuner {

template<typename Impl, typename T> struct SampleCollectingTuner {
    using ConfigImpl = T;

    const u16 samples_total;

    explicit SampleCollectingTuner(ConfigImpl &config, u16 samples) noexcept :
        _config{config}, samples_total{samples} {}

    [[nodiscard]] bool running() const noexcept { return _state != State::Idle; }

    void start() noexcept {
        _state = State::Processing;
     
        _samples_processed = 0;
        impl().onStartImpl();
    }

    void poll(typename ConfigImpl::SampleType sample) noexcept {
        switch (_state) {
            case State::Idle://
                return;

            case State::Processing: {
                impl().onSampleImpl(sample);
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

private:
    ConfigImpl &_config;
    u16 _samples_processed{0};

    enum class State : u8 {
        Idle,
        Processing,
        Calculating,
    } _state{State::Idle};

    // CRTP

    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf
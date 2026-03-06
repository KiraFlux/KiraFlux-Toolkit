// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"

namespace kf {

template<typename Impl, typename T> struct Tuner {
private:
    T &_config;

public:
    const u16 samples_total;

private:
    u16 _samples_processed{0};

    enum class State : u8 {
        Idle,
        Collecting,
        Calculating,
    } _state{State::Idle};

public:
    explicit Tuner(T &config, u16 samples) noexcept :
        _config{config}, samples_total{samples} {}

    [[nodiscard]] bool running() const noexcept { return _state != State::Idle; }

    void start() noexcept {
        _samples_processed = 0;
        _state = State::Collecting;
        impl().onStart();
    }

    void poll(typename T::tuned_type sample) noexcept {
        switch (_state) {
            case State::Idle://
                return;

            case State::Collecting: {
                impl().onSample(sample);
                _samples_processed += 1;
                if (_samples_processed >= samples_total) {
                    _state = State::Calculating;
                }
            }
                return;

            case State::Calculating: {
                implConst().calculate(_config);
                _state = State::Idle;
            }
                return;
        }
    }

private:
    inline Impl &impl() noexcept { return *static_cast<Impl *>(this); }

    inline const Impl &implConst() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
#include "kf/core/attributes.hpp"


namespace kf {

template<typename Impl, typename T> struct Tuner {
private:
    T &config;

public:
    const u16 samples_total;

private:
    u16 samples_processed{0};

    enum class State : u8 {
        Idle,
        Collecting,
        Calculating,
    } state{State::Idle};

public:

    explicit Tuner(T &config, u16 samples) noexcept:
        config{config}, samples_total{samples} {}

    kf_nodiscard bool running() const noexcept { return state != State::Idle; }

    void start() noexcept {
        samples_processed = 0;
        state = State::Collecting;
        impl().onStart();
    }

    void poll(typename T::tuned_type sample) noexcept {
        switch (state) {
            case State::Idle://
                return;

            case State::Collecting: {
                impl().onSample(sample);
                samples_processed += 1;
                if (samples_processed >= samples_total) {
                    state = State::Calculating;
                }
            }
                return;

            case State::Calculating: {
                c_impl().calculate(config);
                state = State::Idle;
            }
                return;
        }
    }

private:
    inline Impl &impl() noexcept { return *static_cast<Impl *>(this); }

    inline const Impl &c_impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}
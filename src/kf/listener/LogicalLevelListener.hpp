// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/listener/Listener.hpp"
#include "kf/math.hpp"
#include "kf/mixin/Configurable.hpp"

namespace kf::internal {

struct LogicalLevelListenerConfig final {
    math::Milliseconds debounce;
};

}// namespace kf::internal

namespace kf::listener {

/// @brief Minimal button with press detection only
struct LogicalLevelListener :

    Listener<LogicalLevelListener, bool, void()>,
    mixin::Configurable<internal::LogicalLevelListenerConfig>

{
    using Config = internal::LogicalLevelListenerConfig;

    using mixin::Configurable<Config>::Configurable;

private:
    math::Milliseconds _next{0};
    bool _last_stable{false};
    bool _last_raw{false};
    bool _click_ready{false};
    bool _first{true};

    KF_IMPL_RESETTABLE(LogicalLevelListener);
    void resetImpl() noexcept {
        _first = true;
        _last_stable = _last_raw = _click_ready = false;
    }

    KF_IMPL_TIMED_POLLABLE(LogicalLevelListener);
    void pollImpl(math::Milliseconds now) noexcept {
        if (this->value().isNone()) { return; }

        const bool state = this->value().unwrap();

        if (_first) {
            _first = false;
            _last_raw = state;
            _last_stable = state;
        }

        if (state != _last_raw) {
            _last_raw = state;
            _next = now + this->config().debounce;
        }

        if (now >= _next) {
            if (_last_stable != state) {
                _last_stable = state;

                if (_last_stable) {
                    _click_ready = true;
                }
            }
        }

        if (_click_ready) {
            _click_ready = false;
            this->invoke();
        }
    }
};

}// namespace kf::listener
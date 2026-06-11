// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "kf/gpio/GPIO.hpp"
#include "kf/math/units.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/TimedPollable.hpp"

namespace kf::internal {

struct LogicalLevelListenerConfig final {
    math::Milliseconds debounce;
};

}// namespace kf::internal

namespace kf::input {

/// @brief Minimal button with press detection only
/// @tparam G Implementation of GPIO with digial INPUT support
template<typename G> struct LogicalLevelListener :

    mixin::NonCopyable,
    mixin::Initable<LogicalLevelListener<G>, void>,
    mixin::Configurable<internal::LogicalLevelListenerConfig>,
    mixin::TimedPollable<LogicalLevelListener<G>>

{
    KF_CHECK_IMPL(G, ::kf::gpio::GPIO::DigitalInputTag);

    using DigitalInputImpl = G;
    using Config = internal::LogicalLevelListenerConfig;

    explicit LogicalLevelListener(const Config &config, DigitalInputImpl &&pin) noexcept :
        mixin::Configurable<Config>{config}, _pin{std::move(pin)} {}

    /// @brief Check if button was clicked (consumes the click)
    /// @return true if button was pressed since last call
    [[nodiscard]] bool clicked() noexcept {
        if (_click_ready) {
            _click_ready = false;
            return true;
        }
        return false;
    }

    /// @brief Check current button state
    /// @return true if button is currently pressed (after debounce)
    [[nodiscard]] bool pressed() const noexcept {
        return _last_stable;
    }

private:
    math::Milliseconds _next{0};
    bool _last_stable{false};
    bool _last_raw{false};
    bool _click_ready{false};
    bool _first{true};
    DigitalInputImpl _pin;

    // impl
    using This = LogicalLevelListener<G>;

    KF_IMPL_INITABLE(This, void);
    void initImpl() noexcept {
        _pin.init();
    }

    KF_IMPL_TIMED_POLLABLE(This);
    void pollImpl(math::Milliseconds now) noexcept {
        const bool state = _pin.read();

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
    }
};

}// namespace kf::input
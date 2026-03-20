// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

#include "kf/aliases.hpp"
#include "kf/gpio/GPIO.hpp"
#include "kf/math/units.hpp"
#include "kf/meta/type_check.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/Initable.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/TimedPollable.hpp"

namespace kf::input {

namespace internal {

struct ButtonConfig {
    math::Milliseconds debounce;
};

}// namespace internal

/// @brief Minimal button with press detection only
template<typename I> struct Button : mixin::Initable<Button<I>, void>, mixin::NonCopyable, mixin::TimedPollable<Button<I>>, mixin::Configurable<internal::ButtonConfig> {
    kf_crtp_check(I, kf::gpio::DigitalInputTag);

    using PinImpl = I;
    using Config = internal::ButtonConfig;

    explicit Button(const Config &config, PinImpl &&pin) noexcept :
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
    [[nodiscard]] bool pressed() const noexcept { return _last_stable; }

private:
    math::Milliseconds _next{0};
    PinImpl _pin;
    bool _last_stable{false};
    bool _last_raw{false};
    bool _click_ready{false};

    // impl
    using This = Button<I>;

    KF_IMPL_INITABLE(This, void);
    void initImpl() noexcept { _pin.init(); }

    KF_IMPL_TIMED_POLLABLE(This);
    void pollImpl(math::Milliseconds now) noexcept {
        const bool state = _pin.read();

        // todo use Timer here

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
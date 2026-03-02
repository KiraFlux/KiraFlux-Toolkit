// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/math/units.hpp"

namespace kf::math {

struct Timer final {

private:
    Milliseconds _period, _last{0};

public:
    constexpr explicit Timer(Milliseconds period) noexcept : _period{period} {}

    constexpr explicit Timer(Hertz frequency) noexcept : _period{static_cast<Milliseconds>(1000u / frequency)} {}

    // propetries

    /// @brief Set new timer period
    void period(Milliseconds new_period) noexcept { _period = new_period; }

    /// @brief Get current timer period
    [[nodiscard]] Milliseconds period() const noexcept { return _period; }

    // control

    /// @brief Start (restart) timer
    void restart(Milliseconds now) noexcept { _last = now; }

    /// @brief Expired check
    /// @note Do not automaticly reset
    [[nodiscard]] bool expired(Milliseconds now) noexcept { return elapsed(now) >= _period; }

    /// @brief Get Time since last start()
    [[nodiscard]] Milliseconds elapsed(Milliseconds now) noexcept { return now - _last; }

    /// @brief Get Time before Timer expire
    [[nodiscard]] Milliseconds remaining(Milliseconds now) noexcept {
        const auto e = elapsed(now);
        if (e < _period) {// because of unsigned arithmetic
            return _period - e;
        } else {
            return 0;
        }
    }
};

}// namespace kf::math

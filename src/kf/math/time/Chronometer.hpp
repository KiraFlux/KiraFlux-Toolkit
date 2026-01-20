// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/math/units.hpp"

namespace kf {

/// @brief Chronometer for measuring time intervals between calls
/// @note Uses microsecond precision for accurate delta time calculations
struct Chronometer final {

private:
    // todo убрать зависимость от micros()
    Microseconds last{micros()};///< Timestamp of last measurement

public:
    /// @brief Calculate time delta since last call
    /// @param now Current time in microseconds
    /// @return Time elapsed in seconds since last calc() call
    /// @note Updates internal timestamp for next calculation
    Seconds calc(Microseconds now) noexcept {
        const auto delta = now - last;
        last = now;
        return static_cast<Seconds>(delta) * 1e-6f;
    }
};

}// namespace kf
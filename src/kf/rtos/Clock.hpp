// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    rtos/Clock.hpp
/// @brief   Provides monotonic time in milliseconds.

#pragma once

#include "kf/units.hpp"

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <time.h>
#include <unistd.h>
#endif

namespace kf::rtos {

struct Clock {

    /// @brief Get current time in Milliseconds
    [[nodiscard]] static units::Milliseconds now() noexcept {
#ifdef ARDUINO
        return ::millis();
#else
        struct timespec ts;
        ::clock_gettime(CLOCK_MONOTONIC, &ts);

        return units::Milliseconds{
            static_cast<u32>((static_cast<u64>(ts.tv_sec) * 1000) + (static_cast<u64>(ts.tv_nsec) / 1'000'000)),
        };
#endif
    }
};

}// namespace kf::rtos
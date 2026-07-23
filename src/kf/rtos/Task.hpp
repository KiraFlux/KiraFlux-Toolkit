// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/units.hpp"

#ifdef ARDUINO
#include <Arduino.h>
#else
#include <unistd.h>
#endif

namespace kf::rtos {

struct Task {

    static void sleep(units::Milliseconds duration) noexcept {
#ifdef ARDUINO
        ::delay(duration);
#else
        (void) ::usleep(static_cast<useconds_t>(duration) * 1000);
#endif
    }
};

}// namespace kf::rtos
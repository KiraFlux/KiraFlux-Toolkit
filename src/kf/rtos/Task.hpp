// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/units.hpp"

#ifdef ARDUINO
#include <Arduino.h>
#endif

namespace kf::rtos {

struct Task {

    static void sleep(units::Milliseconds duration) noexcept {
#ifdef ARDUINO
        ::delay(duration);
#endif
    }
};

}// namespace kf::rtos
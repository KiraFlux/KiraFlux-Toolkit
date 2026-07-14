// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <Arduino.h>

#include "kf/rtos/Task.hpp"
#include "kf/units.hpp"

namespace kf::arduino {

struct ArduinoTask : rtos::Task<ArduinoTask> {
private:
    KF_IMPL_TASK(ArduinoTask);
    static void sleepImpl(units::Milliseconds duration) noexcept {
        ::delay(duration);
    }
};

}// namespace kf::arduino

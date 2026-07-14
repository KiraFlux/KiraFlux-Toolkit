// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/meta/CRTP.hpp"
#include "kf/units.hpp"

namespace kf::rtos {

struct TaskTag {};

template<typename Impl> struct Task :

    TaskTag,
    meta::CRTP<Impl>

{

    static void sleep(units::Milliseconds duration) noexcept {
        Impl::sleepImpl(duration);
    }
};

}// namespace kf::rtos

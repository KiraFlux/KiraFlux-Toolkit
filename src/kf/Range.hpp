// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/algorithm.hpp"

namespace kf {

template<typename T> struct Range {
    T start;
    T end;

    [[nodiscard]] constexpr T clamped(T value) const noexcept {
        return clamp(value, start, end);
    }
};

}// namespace kf

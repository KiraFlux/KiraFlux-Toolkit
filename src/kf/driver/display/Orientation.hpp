// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    driver/display/Orientation.hpp
/// @brief   Enum for display orientation (normal, mirror, rotate).

#pragma once

namespace kf::driver::display {

/// @brief Display orientation modes
enum class Orientation : unsigned char {
    Normal = 0,          ///< Default orientation
    MirrorX = 1,         ///< Horizontal mirror
    MirrorY = 2,         ///< Vertical mirror
    Flip = 3,            ///< 180-degree rotation
    ClockWise = 4,       ///< 90-degree clockwise rotation
    CounterClockWise = 5,///< 90-degree counterclockwise rotation
};

}// namespace kf::driver::display

// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"


namespace kf {

/// @brief Pixel format enumeration
enum class PixelFormat : u8 {
    Monochrome,///< 1-bit monochrome format (1 bit per pixel)
    RGB565,    ///< 16-bit BIG ENDIAN RGB565 format (5-6-5 bits per channel)
};

}// namespace kf
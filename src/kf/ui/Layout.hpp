// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::ui {

/// @brief Semantic layout hints for arranging widgets on a page
///
/// @note
/// A page can suggest a preferred layout to the renderer.
/// The renderer is free to ignore or adapt the suggestion based on display capabilities and available space.
/// Fallback to `Vertical` is recommended when the requested layout is not supported.
enum class Layout : char {

    /// @brief Vertical list (one widget per line), the default fallback.
    /// @note Supports scrolling and works on any display.
    Vertical,

    /// @brief Horizontal row (widgets placed side‑by‑side)
    /// @note Useful for small sets of options (e.g., 2‑3 buttons)
    Horizontal,

    /// @brief Grid with N columns and M rows (up to N*M widgets)
    Grid2x2,
    Grid3x2,
    Grid2x3,
    Grid3x3
};

}// namespace kf::ui
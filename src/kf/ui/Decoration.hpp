// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    ui/Decoration.hpp
/// @brief   Semantic decoration elements (arrows, colon, space).

#pragma once

namespace kf::ui {

/// @brief Semantic decoration elements for UI layout
/// @note Decoration are small visual markers (arrows, separators, etc.)
///       that carry meaning independent of the actual renderer.
///       The renderer maps each decoration to an appropriate symbol or glyph.
///       For monochrome or limited displays, the renderer may substitute with a simple character.
enum class Decoration : char {
    /// @brief Horizontal space between primitives
    Space,

    /// @brief Separator between a label and its value (e.g., "Name: value")
    Colon,

    /// @brief Indicates backward navigation, collapse, or "go back" action
    LongArrowLeft,

    /// @brief Indicates forward navigation, expansion, or "go to" action
    LongArrowRight,

    /// @brief Directional arrows
    ShortArrowLeft,
    ShortArrowRight,
    ShortArrowUp,
    ShortArrowDown
};

}// namespace kf::ui
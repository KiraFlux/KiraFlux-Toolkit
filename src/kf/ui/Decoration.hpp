// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::ui {

/// @brief Semantic decoration elements for UI layout
/// @note Decoration are small visual markers (arrows, separators, etc.)
///       that carry meaning independent of the actual renderer.
///       The renderer maps each decoration to an appropriate symbol or glyph.
///       For monochrome or limited displays, the renderer may substitute with a simple character.
enum class Decoration : char {
    Space = 0x00,///< Horizontal space between primitives
    Arrow = 0x01,///< Indicates navigation, expansion, or "go to" action.
    Colon = 0x02,///< Separator between a label and its value (e.g., "Name: value").
};

}// namespace kf::ui
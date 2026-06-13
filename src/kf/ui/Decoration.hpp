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
    Arrow,///< Indicates navigation, expansion, or "go to" action.
    Colon,///< Separator between a label and its value (e.g., "Name: value").
};

}// namespace kf::ui
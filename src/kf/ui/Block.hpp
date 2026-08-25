// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    ui/Block.hpp
/// @brief   Enum for semantic block types (standard, alternative).

#pragma once

namespace kf::ui {

/// @brief Semantic block type for UI layout and grouping.
/// @note Blocks define visual containers (e.g., borders, background, spacing) independent of the actual renderer.
/// The renderer maps each block type to an appropriate visual representation.
/// For monochrome or limited displays, the renderer may ignore or substitute styles.
enum class Block : char {
    Standard,   ///< Standard content block (default appearance)
    Alternative,///< Visually distinct block (e.g., inset, highlight, secondary)
};

}// namespace kf::ui
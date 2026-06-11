// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::ui {

/// @brief Semantic color tags for UI rendering.
/// @note These colors carry meaning independent of the actual display hardware.
///
///       The concrete renderer maps each semantic color to an appropriate visual representation.
///
///       For monochrome or limited displays, the renderer may ignore or substitute colors.
enum class Color : unsigned char {

    /// @brief Default text/foreground color
    /// @note For foreground: standard content text (e.g., labels, values). Should have high contrast with background.
    /// @note For background: default background of UI panels or pages.
    Normal = 0x00,

    /// @brief Main accent color, used for primary actions or highlights
    /// @note For foreground: important buttons, active links, selected items.
    /// @note For background: rarely used; prefer using Primary for foreground only.
    Primary = 0x01,

    /// @brief Secondary accent, less prominent than Primary
    /// @note For foreground: secondary buttons, less important links, disabled state text.
    /// @note For background: subtle highlights or borders.
    Secondary = 0x02,

    /// @brief Indicates a successful operation or positive status
    /// @note For foreground: success messages, confirmation text, online status.
    /// @note For background: success alerts or badges (ensure text contrast).
    Success = 0x03,

    /// @brief Indicates a warning or caution
    /// @note For foreground: warning messages, non‑critical errors.
    /// @note For background: warning banners or fields requiring attention.
    Warning = 0x04,

    /// @brief Indicates an error or critical issue
    /// @note For foreground: error messages, destructive actions, invalid input.
    /// @note For background: error alerts, red badges (text should be readable).
    Error = 0x05,

    /// @brief Neutral informational content
    /// @note For foreground: help text, tooltips, info panels.
    /// @note For background: information boxes, notifications.
    Info = 0x06,

    /// @brief Indicates an inactive or disabled element
    /// @note For foreground: disabled buttons, inactive menu items, placeholders.
    /// @note For background: disabled controls or read‑only fields.
    Disabled = 0x07
};

}// namespace kf::ui
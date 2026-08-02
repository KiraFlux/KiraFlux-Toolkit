// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    ui/Request.hpp
/// @brief   Enum for widget action requests (Nothing, Redraw, Rebuild).

#pragma once

namespace kf::ui {

/// @brief Action requested by a widget after handling user input
/// @note Widgets return one of these values to tell the page what to do next.
/// @note This allows fine-grained control over UI updates without unnecessary work.
enum class Request : char {

    /// @brief Nothing needs to be done
    /// @note Used when the widget's state didn't change (e.g., clicking a disabled button).
    Nothing,

    /// @brief The widget's visual appearance changed and needs redrawing
    /// @note Used for value changes (slider, checkbox) that don't affect the layout.
    Redraw,

    /// @brief The widget list on the page changed and needs rebuilding
    /// @note Used when widgets are added or removed (e.g., expanding a fold, switching tabs).
    Rebuild,
};

}// namespace kf::ui
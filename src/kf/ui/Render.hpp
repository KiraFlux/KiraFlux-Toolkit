// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"
#include "kf/memory/StringView.hpp"


namespace kf {// NOLINT(*-concat-nested-namespaces) // for c++11 capability

/// @brief External UI components namespace
namespace ui {

/// @brief CRTP base class for UI rendering systems
/// @tparam Impl Concrete renderer implementation type
/// @note Delegates all rendering operations to derived class implementation
template<typename Impl> struct Render {
    friend Impl;

    // Control operations

    /// @brief Prepare render buffer for new frame
    void prepare() { impl().prepareImpl(); }

    /// @brief Finalize frame after rendering
    void finish() { impl().finishImpl(); }

    /// @brief Begin rendering specific widget
    /// @param index Widget index in UI hierarchy
    void beginWidget(usize index) { impl().beginWidgetImpl(index); }

    /// @brief Finish rendering current widget
    void endWidget() { impl().endWidgetImpl(); }

    /// @brief Get remaining widget rendering capacity
    /// @return Number of widgets that can still be rendered in current frame
    kf_nodiscard usize widgetsAvailable() { return impl().widgetsAvailableImpl(); }

    // Value rendering

    /// @brief Render page title
    /// @param title Title text string
    void title(StringView title) { impl().titleImpl(title); }

    /// @brief Render text string
    /// @param str String to display
    void string(StringView str) { impl().stringImpl(str); }

    /// @brief Render integer number
    /// @param integer Integer value to display
    void number(i32 integer) { impl().numberImpl(integer); }

    /// @brief Render floating-point number
    /// @param real Floating-point value to display
    /// @param rounding Number of decimal places to show
    void number(f64 real, u8 rounding) { impl().numberImpl(real, rounding); }

    // Decoration and layout

    /// @brief Render arrow pointing from edge to widget
    void arrow() { impl().arrowImpl(); }

    /// @brief Render colon separator
    void colon() { impl().colonImpl(); }

    /// @brief Begin contrasting text region (higher visibility)
    void beginContrast() { impl().beginContrastImpl(); }

    /// @brief End contrasting text region
    void endContrast() { impl().endContrastImpl(); }

    /// @brief Begin standard content block
    void beginBlock() { impl().beginBlockImpl(); }

    /// @brief End standard content block
    void endBlock() { impl().endBlockImpl(); }

    /// @brief Begin alternative content block (different styling)
    void beginAltBlock() { impl().beginAltBlockImpl(); }

    /// @brief End alternative content block
    void endAltBlock() { impl().endAltBlockImpl(); }

private:
    /// @brief Get reference to derived implementation
    /// @return Reference to concrete renderer instance
    inline Impl &impl() { return *static_cast<Impl *>(this); }
};

}// namespace ui
}// namespace kf
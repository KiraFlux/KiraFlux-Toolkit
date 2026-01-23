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

    using Base = Render;

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

    /// @brief Render checkbox
    void checkbox(bool enabled) { impl().checkboxImpl(enabled); }

    /// @brief Render value
    /// @param value Value to display
    template<typename T> void value(T value) { impl().valueImpl(value); }

    // Decoration and layout

    /// @brief Render arrow pointing from edge to widget
    void arrow() { impl().arrowImpl(); }

    /// @brief Render colon separator
    void colon() { impl().colonImpl(); }

    /// @brief Begin contrasting text region (higher visibility)
    void beginFocused() { impl().beginFocusedImpl(); }

    /// @brief End contrasting text region
    void endFocused() { impl().endFocusedImpl(); }

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
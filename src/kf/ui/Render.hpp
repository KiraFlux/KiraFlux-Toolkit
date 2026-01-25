// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
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
    void prepare() noexcept { impl().prepareImpl(); }

    /// @brief Finalize frame after rendering
    void finish() noexcept { impl().finishImpl(); }

    /// @brief Begin rendering specific widget
    /// @param index Widget index in UI hierarchy
    void beginWidget(usize index) noexcept { impl().beginWidgetImpl(index); }

    /// @brief Finish rendering current widget
    void endWidget() noexcept { impl().endWidgetImpl(); }

    /// @brief Get remaining widget rendering capacity
    /// @return Number of widgets that can still be rendered in current frame
    kf_nodiscard usize widgetsAvailable() noexcept { return impl().widgetsAvailableImpl(); }

    // Value rendering

    /// @brief Render page title
    /// @param title Title text string
    void title(StringView title) noexcept { impl().titleImpl(title); }

    /// @brief Render checkbox
    void checkbox(bool enabled) noexcept { impl().checkboxImpl(enabled); }

    /// @brief Render value
    /// @param value Value to display
    template<typename T> void value(T value) noexcept { impl().valueImpl(value); }

    // Decoration and layout

    /// @brief Render arrow pointing from edge to widget
    void arrow() noexcept { impl().arrowImpl(); }

    /// @brief Render colon separator
    void colon() noexcept { impl().colonImpl(); }

    /// @brief Begin contrasting text region (higher visibility)
    void beginFocused() noexcept { impl().beginFocusedImpl(); }

    /// @brief End contrasting text region
    void endFocused() noexcept { impl().endFocusedImpl(); }

    /// @brief Begin standard content block
    void beginBlock() noexcept { impl().beginBlockImpl(); }

    /// @brief End standard content block
    void endBlock() noexcept { impl().endBlockImpl(); }

    /// @brief Begin alternative content block (different styling)
    void beginAltBlock() noexcept { impl().beginAltBlockImpl(); }

    /// @brief End alternative content block
    void endAltBlock() noexcept { impl().endAltBlockImpl(); }

private:
    /// @brief Get reference to derived implementation
    /// @return Reference to concrete renderer instance
    inline Impl &impl() noexcept { return *static_cast<Impl *>(this); }
};

}// namespace ui
}// namespace kf
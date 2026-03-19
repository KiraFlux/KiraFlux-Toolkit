// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/mixin/NonCopyable.hpp"

#include "kf/ui/internal/ValuePlacement.hpp"

namespace kf::ui::render {

struct RenderTag {};

/// @brief CRTP base class for UI rendering systems
/// @tparam Impl Concrete renderer implementation type
/// @note Delegates all rendering operations to derived class implementation
template<typename Impl> struct Render : RenderTag, mixin::NonCopyable {
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
    [[nodiscard]] usize widgetsAvailable() const noexcept { return impl().widgetsAvailableImpl(); }

    // Value rendering

    /// @brief Render page title
    void title(memory::StringView title) noexcept { impl().titleImpl(title); }

    /// @brief Render checkbox
    void checkbox(bool enabled) noexcept { impl().checkboxImpl(enabled); }

    /// @brief Render slider
    template<typename T> void slider(
        T v, T min_value, T max_value,
        internal::ValuePlacement value_placement) noexcept {
        impl().sliderImpl(v, min_value, max_value, value_placement);
    }

    /// @brief Render value
    template<typename T> void value(T v) noexcept { impl().valueImpl(v); }

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
    Impl &impl() noexcept { return *static_cast<Impl *>(this); }
    const Impl &impl() const noexcept { return *static_cast<const Impl *>(this); }
};

}// namespace kf::ui::render
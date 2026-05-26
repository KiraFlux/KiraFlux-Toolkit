// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Range.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/primitives.hpp"

#include "kf/ui/internal/Placement.hpp"

namespace kf::ui::render {

struct RenderTag {};

/// @brief CRTP base class for UI rendering systems
/// @tparam Impl Concrete renderer implementation type
/// @note Delegates all rendering operations to derived class implementation
template<typename Impl> struct Render : RenderTag, mixin::NonCopyable, meta::CRTP<Impl> {
    // Control operations

    /// @brief Prepare render buffer for new frame
    void prepare() noexcept { this->impl().prepareImpl(); }

    /// @brief Finalize frame after rendering
    void finish() noexcept { this->impl().finishImpl(); }

    /// @brief Begin rendering specific widget
    /// @param index Widget index in UI hierarchy
    void beginWidget(usize index) noexcept { this->impl().beginWidgetImpl(index); }

    /// @brief Finish rendering current widget
    void endWidget() noexcept { this->impl().endWidgetImpl(); }

    /// @brief Get remaining widget rendering capacity
    /// @return Number of widgets that can still be rendered in current frame
    [[nodiscard]] usize widgetsAvailable() const noexcept { return this->impl().widgetsAvailableImpl(); }

    // Value rendering

    /// @brief Render page title
    void title(memory::StringView title) noexcept { this->impl().titleImpl(title); }

    /// @brief Render checkbox
    void checkbox(bool enabled) noexcept { this->impl().checkboxImpl(enabled); }

    /// @brief Render slider
    template<typename T> void slider(T v, Range<T> value_range, internal::Placement placement) noexcept {
        this->impl().sliderImpl(v, value_range, placement);
    }

    /// @brief Render value
    template<typename T> void value(T v) noexcept { this->impl().valueImpl(v); }

    // Decoration and layout

    /// @brief Render arrow pointing from edge to widget
    void arrow() noexcept { this->impl().arrowImpl(); }

    /// @brief Render colon separator
    void colon() noexcept { this->impl().colonImpl(); }

    /// @brief Begin contrasting text region (higher visibility)
    void beginFocused() noexcept { this->impl().beginFocusedImpl(); }

    /// @brief End contrasting text region
    void endFocused() noexcept { this->impl().endFocusedImpl(); }

    /// @brief Begin standard content block
    void beginBlock() noexcept { this->impl().beginBlockImpl(); }

    /// @brief End standard content block
    void endBlock() noexcept { this->impl().endBlockImpl(); }

    /// @brief Begin alternative content block (different styling)
    void beginAltBlock() noexcept { this->impl().beginAltBlockImpl(); }

    /// @brief End alternative content block
    void endAltBlock() noexcept { this->impl().endAltBlockImpl(); }
};

}// namespace kf::ui::render
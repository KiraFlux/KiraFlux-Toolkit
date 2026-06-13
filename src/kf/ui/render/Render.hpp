// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include "kf/Option.hpp"
#include "kf/Range.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/StringRepresentable.hpp"
#include "kf/primitives.hpp"

#include "kf/ui/Block.hpp"
#include "kf/ui/Color.hpp"
#include "kf/ui/Decoration.hpp"
#include "kf/ui/Placement.hpp"
#include "kf/ui/Style.hpp"

namespace kf::ui::render {

struct RenderTag {};

/// @brief CRTP base class for UI rendering systems
/// @tparam Impl Concrete renderer implementation type
/// @note Delegates all rendering operations to derived class implementation
template<typename Impl> struct Render :

    RenderTag,
    meta::CRTP<Impl>,
    mixin::NonCopyable

{
    // Control operations

    /// @brief Prepare render buffer for new frame
    void beginFrame() noexcept {
        this->impl().beginFrameImpl();
    }

    /// @brief Finalize frame after rendering
    void endFrame() noexcept {
        this->impl().endFrameImpl();
    }

    /// @brief Begin rendering specific widget
    /// @param index Widget position in it's page
    /// @param is_focused contrasting text region (higher visibility)
    /// @param style Widget's style
    void beginWidget(usize index, bool is_focused, const Style &style) noexcept {
        this->impl().beginWidgetImpl(index, is_focused, style);
    }

    /// @brief Finish rendering current widget
    void endWidget() noexcept {
        this->impl().endWidgetImpl();
    }

    /// @brief Get remaining widget rendering capacity
    /// @return Number of widgets that can still be rendered in current frame
    [[nodiscard]] usize widgetsAvailable() const noexcept {
        return this->impl().widgetsAvailableImpl();
    }

    // Value rendering

    /// @brief Render page title
    void title(memory::StringView title) noexcept {
        this->impl().titleImpl(title);
    }

    /// @brief Render checkbox
    void checkbox(bool enabled) noexcept {
        this->impl().checkboxImpl(enabled);
    }

    /// @brief Render slider
    template<typename T> void slider(const T &v, const Range<T> &range, Placement placement) noexcept {
        this->impl().sliderImpl(v, range, placement);
    }

    /// @brief Render value
    template<typename T> void value(const T &v) noexcept {
        if constexpr (std::is_base_of_v<mixin::StringRepresentableTag, T>) {
            this->value(v.toString());
        } else if constexpr (std::is_base_of_v<OptionTag, T>) {

            if (v.isSome()) {
                this->value(v.unwrap());
            } else {
                this->impl().valueImpl(none);
            }

        } else {
            this->impl().valueImpl(v);
        }
    }

    // Semantic color

    /// @brief Set foreground Semantic color
    void foreground(Color color) noexcept {
        this->impl().setForegroundImpl(color);
    }

    /// @brief Set background Semantic color
    void background(Color color) noexcept {
        this->impl().setBackgroundImpl(color);
    }

    // Decoration and layout

    /// @brief Render a semantic decoration element
    /// @param decoration The type of decoration to render
    /// @note Decorations are lightweight markers that help users interpret the UI.
    void decoration(Decoration decoration) noexcept {
        this->impl().decorationImpl(decoration);
    }

    /// @brief Begin content block
    void beginBlock(Block block_type = Block::Standard) noexcept {
        this->impl().beginBlockImpl(block_type);
    }

    /// @brief End content block
    void endBlock(Block block_type = Block::Standard) noexcept {
        this->impl().endBlockImpl(block_type);
    }
};

}// namespace kf::ui::render
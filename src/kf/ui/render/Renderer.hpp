// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include "kf/Option.hpp"
#include "kf/Range.hpp"
#include "kf/StringView.hpp"
#include "kf/meta/CRTP.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/StringRepresentable.hpp"
#include "kf/primitives.hpp"

#include "kf/ui/Block.hpp"
#include "kf/ui/Color.hpp"
#include "kf/ui/Decoration.hpp"
#include "kf/ui/Layout.hpp"

namespace kf::ui::render {

struct RenderTag {};

/// @brief CRTP base class for UI rendering systems
/// @tparam Impl Concrete renderer implementation type
/// @note Delegates all rendering operations to derived class implementation
template<typename Impl> struct Renderer :

    RenderTag,
    meta::CRTP<Impl>,
    mixin::NonCopyable

{
    /// @brief Check whether a render has been requested
    /// @return `true` if a render is pending, `false` otherwise
    [[nodiscard]] constexpr bool renderRequested() noexcept {
        return _render_requested;
    }

    /// @brief Request a full UI redraw on the next poll cycle
    /// @note The actual rendering will be performed during the next call to `poll()`
    void requestRender() noexcept {
        _render_requested = true;
    }

    // Control operations

    /// @brief Prepare render buffer for new frame
    void beginFrame() noexcept {
        this->impl().beginFrameImpl();
    }

    /// @brief Finalize frame after rendering
    void endFrame() noexcept {
        this->impl().endFrameImpl();
        _render_requested = false;
    }

    /// @brief Begin rendering page
    /// @param title Page title
    /// @param layout Page layout hint
    /// @return Number of widgets that can still be rendered in current frame
    [[nodiscard]] usize beginPage(StringView title, Layout layout) noexcept {
        return this->impl().beginPageImpl(title, layout);
    }

    /// @brief Finish rendering page
    void endPage() noexcept {
        this->impl().endPageImpl();
    }

    /// @brief Begin rendering specific widget
    /// @param index Widget position on page
    /// @param is_focused contrasting text region (higher visibility)
    void beginWidget(usize index, bool is_focused) noexcept {
        this->impl().beginWidgetImpl(index, is_focused);
    }

    /// @brief Finish rendering current widget
    void endWidget() noexcept {
        this->impl().endWidgetImpl();
    }

    /// @brief Begin content block
    void beginBlock(Block block_type = Block::Standard) noexcept {
        this->impl().beginBlockImpl(block_type);
    }

    /// @brief End content block
    void endBlock(Block block_type = Block::Standard) noexcept {
        this->impl().endBlockImpl(block_type);
    }

    // Value rendering

    /// @brief Render a semantic decoration element
    /// @param decoration The type of decoration to render
    /// @note Decorations are lightweight markers that help users interpret the UI.
    void decoration(Decoration decoration) noexcept {
        this->impl().decorationImpl(decoration);
    }

    /// @brief Render checkbox
    void checkbox(bool enabled) noexcept {
        this->impl().checkboxImpl(enabled);
    }

    /// @brief Render slider
    /// @brief fill slider fill value [0..1]
    void slider(f32 fill) noexcept {
        this->impl().sliderImpl(fill);
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

private:
    bool _render_requested{false};
};

}// namespace kf::ui::render
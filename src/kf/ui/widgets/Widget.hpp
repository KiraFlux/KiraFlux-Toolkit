// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Option.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/primitives.hpp"

#include "kf/ui/Color.hpp"
#include "kf/ui/Event.hpp"
#include "kf/ui/render/Render.hpp"

namespace kf::ui::widgets {

struct WidgetTag {};

/// @brief Base widget class for all UI components
/// @note All interactive UI elements inherit from this class
/// @tparam R Render system implementation (Must inherit from `::kf::ui::render::RenderTag`)
/// @tparam E Event type (Must inherit from `::kf::ui::EventTag`)
template<typename R, typename E> struct Widget :

    WidgetTag,
    mixin::NonCopyable

{
    KF_CHECK_IMPL(R, ::kf::ui::render::RenderTag);
    KF_CHECK_IMPL(E, ::kf::ui::EventTag);

    using RenderImpl = R;
    using EventImpl = E;

    /// @brief Widget style
    struct Style {
        Color foreground_color;///< Text color
        Color background_color;///< Background color

        /// @brief Create default style
        [[nodiscard]] static constexpr Style defaults() noexcept {
            return Style{
                .foreground_color = Color::Normal,
                .background_color = Color::Normal,
            };
        }
    };

    /// @brief Render widget content (must be implemented by derived classes)
    virtual void doRender(RenderImpl &render) const noexcept = 0;

    /// @brief Handle click event
    /// @return true if redraw required, false otherwise
    [[nodiscard]] virtual bool onClick() noexcept {
        return false;
    }

    /// @brief Handle Event value
    /// @return true if redraw required, false otherwise
    [[nodiscard]] virtual bool onEventValue(typename EventImpl::Value event_value) noexcept {
        return false;
    }

    explicit constexpr Widget(const Style &style) noexcept : _style{style} {}

    /// @brief Get Contextual hint about this widget
    [[nodiscard]] memory::StringView hint() const noexcept {
        return _hint;
    }

    /// @brief Set Contextual hint about this widget
    void hint(memory::StringView new_hint) noexcept {
        _hint = new_hint;
    }

    /// @brief Get style
    [[nodiscard]] const Style &style() const noexcept {
        return _style;
    }

    /// @brief Set style
    void style(const Style &new_style) noexcept {
        _style = new_style;
    }

    /// @brief Get Foreground color
    [[nodiscard]] Color foreground() const noexcept {
        return _style.foreground_color;
    }

    /// @brief Set Foreground color
    void foreground(Color color) noexcept {
        _style.foreground_color = color;
    }

    /// @brief Get Background color
    [[nodiscard]] Color background() const noexcept {
        return _style.background_color;
    }

    /// @brief Set Background color
    void background(Color color) noexcept {
        _style.background_color = color;
    }

    /// @brief External widget rendering with focus handling
    void render(RenderImpl &render, usize index, bool focused) const noexcept {
        render.beginWidget(index, focused, _style.foreground_color, _style.background_color);
        doRender(render);
        render.endWidget();
    }

private:
    memory::StringView _hint{};
    Style _style{Style::defaultss()};
};

}// namespace kf::ui::widgets
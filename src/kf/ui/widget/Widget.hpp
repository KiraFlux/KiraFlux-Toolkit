// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Option.hpp"
#include "kf/StringView.hpp"
#include "kf/concepts.hpp"
#include "kf/mixin/Styled.hpp"
#include "kf/primitives.hpp"

#include "kf/ui/Color.hpp"
#include "kf/ui/Event.hpp"
#include "kf/ui/Request.hpp"
#include "kf/ui/render/Renderer.hpp"

namespace kf::ui::widget {

struct WidgetTag {};

/// @brief Base widget class for all UI components
/// @note All interactive UI elements inherit from this class
/// @tparam R Render system implementation (Must inherit from `::kf::ui::render::RenderTag`)
/// @tparam E Event type (Must inherit from `::kf::ui::EventTag`)
template<implements<render::RenderTag> R, implements<EventTag> E> struct Widget :

    WidgetTag,
    mixin::NonCopyable,
    mixin::Styled

{
    using RendererImpl = R;
    using EventImpl = E;

    /// @brief Render widget content (must be implemented by derived classes)
    virtual void doRender(RendererImpl &render) const noexcept = 0;

    /// @brief Handle click event
    /// @return Action Request after input
    [[nodiscard]] virtual Request onClick() noexcept {
        return Request::Nothing;
    }

    /// @brief Handle Event value
    /// @return Action Request after input
    [[nodiscard]] virtual Request onEventValue(typename EventImpl::Value event_value) noexcept {
        return Request::Nothing;
    }

    using mixin::Styled::Styled;

    /// @brief Get Widget enabled value
    [[nodiscard]] constexpr bool enabled() const noexcept {
        return _enabled;
    }

    /// @brief Set Widget enabled value
    constexpr void enabled(bool is_enabled) noexcept {
        _enabled = is_enabled;
    }

    /// @brief Enable Widget
    constexpr void enable() noexcept {
        _enabled = true;
    }

    /// @brief Disable Widget
    constexpr void disable() noexcept {
        _enabled = false;
    }

    /// @brief Get Contextual hint about this widget
    [[nodiscard]] constexpr StringView hint() const noexcept {
        return _hint;
    }

    /// @brief Set Contextual hint about this widget
    constexpr void hint(StringView new_hint) noexcept {
        _hint = new_hint;
    }

    /// @brief Get Widget offset
    [[nodiscard]] constexpr usize offset() const noexcept {
        return _offset;
    }

    /// @brief Set Widget offset
    constexpr void offset(usize new_offset) noexcept {
        _offset = static_cast<u8>(new_offset);
    }

    /// @brief External widget rendering with focus handling
    void render(RendererImpl &render, usize index, bool focused) const noexcept {
        render.beginWidget(index, focused, _offset);

        render.foreground(_enabled ? this->style().foreground_color : Color::Disabled);
        render.background(_enabled ? this->style().background_color : Color::Disabled);

        doRender(render);

        render.endWidget();
    }

private:
    StringView _hint{};
    bool _enabled{true};
    u8 _offset{0};
};

}// namespace kf::ui::widget
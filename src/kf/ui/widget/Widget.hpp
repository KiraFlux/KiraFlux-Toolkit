// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Option.hpp"
#include "kf/StringView.hpp"
#include "kf/concepts.hpp"
#include "kf/mixin/Styled.hpp"
#include "kf/primitives.hpp"

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

    // TODO: add enabled/disabled

    /// @brief Get Contextual hint about this widget
    [[nodiscard]] constexpr StringView hint() const noexcept {
        return _hint;
    }

    /// @brief Set Contextual hint about this widget
    constexpr void hint(StringView new_hint) noexcept {
        _hint = new_hint;
    }

    // TODO: add offset(usize)

    /// @brief External widget rendering with focus handling
    void render(RendererImpl &render, usize index, bool focused) const noexcept {
        render.beginWidget(index, focused);
        render.foreground(this->style().foreground_color);
        render.background(this->style().background_color);
        doRender(render);
        render.endWidget();
    }

private:
    StringView _hint{};
};

}// namespace kf::ui::widget
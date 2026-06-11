// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Option.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/primitives.hpp"

#include "kf/ui/Color.hpp"
#include "kf/ui/UiTraits.hpp"

namespace kf::ui::widgets {

struct WidgetTag {};

/// @brief Base widget class for all UI components
/// @note All interactive UI elements inherit from this class
/// @tparam U UI Traits Type
template<typename U> struct Widget :

    WidgetTag,
    mixin::NonCopyable

{
    KF_CHECK_IMPL(U, ::kf::ui::UiTraitsTag);

    /// @brief Render widget content (must be implemented by derived classes)
    virtual void doRender(typename U::RenderImpl &render) const noexcept = 0;

    /// @brief Handle click event
    /// @return true if redraw required, false otherwise
    [[nodiscard]] virtual bool onClick() noexcept {
        return false;
    }

    /// @brief Handle Event value
    /// @return true if redraw required, false otherwise
    [[nodiscard]] virtual bool onEventValue(typename U::EventImpl::Value event_value) noexcept {
        return false;
    }

    /// @brief Get Contextual hint about this widget
    [[nodiscard]] memory::StringView hint() const noexcept {
        return _hint;
    }

    /// @brief Set Contextual hint about this widget
    void hint(memory::StringView new_hint) noexcept {
        _hint = new_hint;
    }

    /// @brief Get Foreground color
    [[nodiscard]] Color foreground() const noexcept {
        return _foreground_color;
    }

    /// @brief Set Foreground color
    void foreground(Color new_color) noexcept {
        _foreground_color = new_color;
    }

    /// @brief Get Background color
    [[nodiscard]] Color background() const noexcept {
        return _background_color;
    }

    /// @brief Set Background color
    void background(Color new_color) noexcept {
        _background_color = new_color;
    }

    /// @brief External widget rendering with focus handling
    void render(typename U::RenderImpl &render, usize index, bool focused) const noexcept {
        render.beginWidget(index, focused, _foreground_color, _background_color);
        doRender(render);
        render.endWidget();
    }

private:
    memory::StringView _hint{};
    Color _foreground_color{Color::Normal}, _background_color{Color::Normal};
};

}// namespace kf::ui::widgets
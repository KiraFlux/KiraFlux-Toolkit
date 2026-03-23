// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/ui/internal/UiTraits.hpp"

namespace kf::ui::widgets {

struct WidgetTag {};

/// @brief Base widget class for all UI components
/// @note All interactive UI elements inherit from this class
template<typename U> struct Widget : WidgetTag, mixin::NonCopyable {
    KF_CHECK_IMPL(U, ::kf::ui::internal::UiTraitsTag);

    /// @brief Construct widget and add to specified page
    /// @param root Page to add widget to
    explicit Widget(typename U::PageImpl &root) {
        root.addWidget(*this);
    }

    /// @brief Default constructor (widget not attached to any page)
    explicit Widget() noexcept = default;

    /// @brief Render widget content (must be implemented by derived classes)
    virtual void doRender(typename U::RenderImpl &render) const noexcept = 0;

    /// @brief Handle click event
    /// @return true if redraw required, false otherwise
    [[nodiscard]] virtual bool onClick() noexcept { return false; }

    /// @brief Handle Event value
    /// @return true if redraw required, false otherwise
    [[nodiscard]] virtual bool onEventValue(typename U::EventImpl::Value event_value) noexcept { return false; }

    /// @brief External widget rendering with focus handling
    void render(typename U::RenderImpl &render, bool focused) const noexcept {
        if (focused) {
            render.beginFocused();
            doRender(render);
            render.endFocused();
        } else {
            doRender(render);
        }
    }
};

}// namespace kf::ui::widgets
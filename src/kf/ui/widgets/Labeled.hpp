// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/StringView.hpp"
#include "kf/mixin/Labeled.hpp"

#include "kf/ui/Decoration.hpp"
#include "kf/ui/Request.hpp"
#include "kf/ui/Style.hpp"
#include "kf/ui/UiTraits.hpp"

namespace kf::ui::widgets {

struct LabeledTag {};

/// @brief Widget wrapper adding label to another widget
/// @tparam U UI Traits Type
template<typename U> struct Labeled :

    LabeledTag,
    U::Widget,
    mixin::Labeled

{
    KF_CHECK_IMPL(U, ::kf::ui::UiTraitsTag);

    explicit constexpr Labeled(StringView label, typename U::Widget &wrapped, Style style = Style::defaults()) noexcept :
        U::Widget{style}, mixin::Labeled{label}, _wrapped{wrapped} {}

    /// @brief Forward click event to wrapped widget
    /// @return Result from wrapped widget's onClick()
    Request onClick() noexcept override {
        return _wrapped.onClick();
    }

    /// @brief Forward change event to wrapped widget
    /// @return Result from wrapped widget's onEventValue()
    Request onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        return _wrapped.onEventValue(event_value);
    }

    void doRender(typename U::RendererImpl &render) const noexcept override {
        render.value(this->label());
        render.decoration(Decoration::Colon);
        _wrapped.doRender(render);
        // restore color
        render.foreground(this->foreground());
        render.background(this->background());
    }

private:
    typename U::Widget &_wrapped;
};

}// namespace kf::ui::widgets
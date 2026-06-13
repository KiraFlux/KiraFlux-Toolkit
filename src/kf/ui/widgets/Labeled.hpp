// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Labeled.hpp"

#include "kf/ui/Styled.hpp"
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

    explicit constexpr Labeled(memory::StringView label, typename U::Widget &wrapped, Styled::Style style = Styled::Style::defaults()) noexcept :
        U::Widget{style}, mixin::Labeled{label}, _wrapped{wrapped} {}

    /// @brief Forward click event to wrapped widget
    /// @return Result from wrapped widget's onClick()
    [[nodiscard]] bool onClick() noexcept override {
        return _wrapped.onClick();
    }

    /// @brief Forward change event to wrapped widget
    /// @return Result from wrapped widget's onEventValue()
    [[nodiscard]] bool onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        return _wrapped.onEventValue(event_value);
    }

    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.value(this->label());
        render.colon();
        _wrapped.doRender(render);
        // restore color
        render.foreground(this->foreground());
        render.background(this->background());
    }

private:
    typename U::Widget &_wrapped;
};

}// namespace kf::ui::widgets
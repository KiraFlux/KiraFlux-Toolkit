// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/ValueCallbacked.hpp"

#include "kf/ui/Request.hpp"
#include "kf/ui/Style.hpp"
#include "kf/ui/UiTraits.hpp"

namespace kf::ui::widgets {

struct CheckBoxTag {};

/// @brief Checkbox widget for boolean input
/// @tparam U UI Traits Type
template<typename U> struct CheckBox :

    CheckBoxTag,
    U::Widget,
    mixin::ValueCallbacked<bool>

{
    KF_CHECK_IMPL(U, ::kf::ui::UiTraitsTag);

    explicit constexpr CheckBox(bool value, Style style = Style::defaults()) noexcept :
        U::Widget{style}, mixin::ValueCallbacked<bool>{value} {}

    Request onClick() noexcept override {
        this->value(not this->value());

        return Request::Redraw;
    }

    Request onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        const bool new_value = (event_value > 0);
        const auto request = (this->value() == new_value) ? Request::Nothing : Request::Redraw;

        this->value(new_value);

        return request;
    }

    void doRender(typename U::RendererImpl &render) const noexcept override {
        render.checkbox(this->value());
    }
};

}// namespace kf::ui::widgets
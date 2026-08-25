// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    ui/widget/Button.hpp
/// @brief   Clickable button with callback.

#pragma once

#include "kf/StringView.hpp"
#include "kf/core.hpp"

#include "kf/mixin/Callbacked.hpp"
#include "kf/mixin/Labeled.hpp"

#include "kf/ui/Request.hpp"
#include "kf/ui/Style.hpp"
#include "kf/ui/UiTraits.hpp"

namespace kf::ui::widget {

struct ButtonTag {};

/// @brief Button widget for triggering actions on click
/// @tparam U UI Traits Type
template<implements<UiTraitsTag> U> struct Button :

    ButtonTag,
    U::Widget,
    mixin::Labeled,
    mixin::Callbacked<void()>

{

    explicit constexpr Button(StringView label, Style style = Style::defaults()) noexcept :
        U::Widget{style}, mixin::Labeled::Labeled{label} {}

    /// @brief Handle button click event
    Request onClick() noexcept override {
        this->invoke();

        return Request::Nothing;
    }

    /// @brief Render button with block styling
    void doRender(typename U::RendererImpl &render) const noexcept override {
        render.beginBlock();
        render.value(this->label());
        render.endBlock();
    }
};

}// namespace kf::ui::widget
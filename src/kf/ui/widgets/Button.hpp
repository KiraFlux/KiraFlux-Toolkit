// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/memory/StringView.hpp"
#include "kf/mixin/Callbacked.hpp"
#include "kf/mixin/Labeled.hpp"

#include "kf/ui/Style.hpp"
#include "kf/ui/UiTraits.hpp"

namespace kf::ui::widgets {

struct ButtonTag {};

/// @brief Button widget for triggering actions on click
/// @tparam U UI Traits Type
template<typename U> struct Button :

    ButtonTag,
    U::Widget,
    mixin::Labeled,
    mixin::Callbacked<>

{
    KF_CHECK_IMPL(U, ::kf::ui::UiTraitsTag);

    explicit constexpr Button(memory::StringView label, Style style = Style::defaults()) noexcept :
        U::Widget{style}, mixin::Labeled::Labeled{label} {}

    /// @brief Handle button click event
    [[nodiscard]] bool onClick() noexcept override {
        this->invoke();
        return false;// button click typically doesn't require redraw
    }

    /// @brief Render button with block styling
    void doRender(typename U::RendererImpl &render) const noexcept override {
        render.beginBlock();
        render.value(this->label());
        render.endBlock();
    }
};

}// namespace kf::ui::widgets
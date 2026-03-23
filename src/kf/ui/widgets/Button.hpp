// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/memory/StringView.hpp"
#include "kf/mixin/Callbacked.hpp"
#include "kf/mixin/Labeled.hpp"

#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui::widgets {

struct ButtonTag {};

/// @brief Button widget for triggering actions on click
template<typename U> struct Button final : ButtonTag, Widget<U>, mixin::Callbacked<void>, mixin::Labeled {
    explicit Button(typename U::PageImpl &root, memory::StringView label) :
        Widget<U>{root}, mixin::Labeled{label} {}

    /// @brief Handle button click event
    [[nodiscard]] bool onClick() noexcept override {
        mixin::Callbacked<void>::invoke();
        return false;// button click typically doesn't require redraw
    }

    /// @brief Render button with block styling
    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.beginBlock();
        render.value(this->label());
        render.endBlock();
    }
};

}// namespace kf::ui::widgets
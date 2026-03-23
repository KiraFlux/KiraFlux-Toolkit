// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/memory/StringView.hpp"
#include "kf/mixin/Handlerable.hpp"

#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui::widgets {

struct ButtonTag {};

/// @brief Button widget for triggering actions on click
template<typename U> struct Button final : ButtonTag, Widget<U>, mixin::Handlerable<void> {
    explicit Button(typename U::PageImpl &root, memory::StringView label) :
        Widget<U>{root}, _label{label} {}

    /// @brief Handle button click event
    [[nodiscard]] bool onClick() noexcept override {
        mixin::Handlerable<void>::invoke();
        return false;// button click typically doesn't require redraw
    }

    /// @brief Render button with block styling
    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.beginBlock();
        render.value(_label);
        render.endBlock();
    }

private:
    memory::StringView _label;///< Button label text
};

}// namespace kf::ui::widgets
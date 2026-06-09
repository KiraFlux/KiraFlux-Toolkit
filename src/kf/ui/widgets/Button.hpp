// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Callbacked.hpp"
#include "kf/mixin/Labeled.hpp"

#include "kf/Option.hpp"
#include "kf/ui/Color.hpp"
#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui::widgets {

struct ButtonTag {};

/// @brief Button widget for triggering actions on click
/// @tparam U UI Traits Type
template<typename U> struct Button :

    ButtonTag,
    Widget<U>,
    mixin::Labeled,
    mixin::Callbacked<>

{
    using mixin::Labeled::Labeled;

    [[nodiscard]] TrivialOption<Color> color() const noexcept {
        return _color;
    }

    void color(TrivialOption<Color> new_color) noexcept {
        _color = new_color;
    }

    /// @brief Handle button click event
    [[nodiscard]] bool onClick() noexcept override {
        this->invoke();
        return false;// button click typically doesn't require redraw
    }

    /// @brief Render button with block styling
    void doRender(typename U::RenderImpl &render) const noexcept override {
        if (_color.isSome()) {
            render.background(_color.unwrap());
        }

        render.beginBlock();
        render.value(this->label());
        render.endBlock();
    }

private:
    TrivialOption<Color> _color{none};
};

}// namespace kf::ui::widgets
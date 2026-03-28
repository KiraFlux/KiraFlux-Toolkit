// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/ValueCallbacked.hpp"

#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui::widgets {

struct CheckBoxTag {};

/// @brief Checkbox widget for boolean input
template<typename U> struct CheckBox final : CheckBoxTag, Widget<U>, mixin::ValueCallbacked<bool> {
    using mixin::ValueCallbacked<bool>::ValueCallbacked;

    [[nodiscard]] bool onClick() noexcept override {
        this->value(not this->value());
        return true;
    }

    [[nodiscard]] bool onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        this->value(event_value > 0);
        return true;
    }

    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.checkbox(this->value());
    }
};

}// namespace kf::ui::widgets
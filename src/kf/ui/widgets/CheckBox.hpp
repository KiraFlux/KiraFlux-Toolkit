// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/ValueCallbacked.hpp"

#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui::widgets {

struct CheckBoxTag {};

/// @brief Checkbox widget for boolean input
template<typename U> struct CheckBox final : CheckBoxTag, Widget<U>, mixin::ValueCallbacked<bool> {
    explicit CheckBox(bool default_state = false) noexcept :
        _state{default_state} {}

    explicit CheckBox(typename U::PageImpl &page, bool default_state = false) :
        Widget<U>{page}, mixin::ValueCallbacked<bool>{default_state} {}

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

private:
    bool _state;
};

}// namespace kf::ui::widgets
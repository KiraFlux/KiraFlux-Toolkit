// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Callbacked.hpp"

#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui::widgets {

struct CheckBoxTag {};

/// @brief Checkbox widget for boolean input
template<typename U> struct CheckBox final : CheckBoxTag, Widget<U>, mixin::Callbacked<bool> {
    explicit CheckBox(bool default_state = false) noexcept :
        _state{default_state} {}

    explicit CheckBox(typename U::PageImpl &root, bool default_state = false) :
        Widget<U>{root}, _state{default_state} {}

    void state(bool new_state) noexcept {
        if (new_state != _state) {
            _state = new_state;
            mixin::Callbacked<bool>::invoke(_state);
        }
    }

    [[nodiscard]] bool state() const noexcept {
        return _state;
    }

    [[nodiscard]] bool onClick() noexcept override {
        state(not _state);
        return true;
    }

    [[nodiscard]] bool onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        state(event_value > 0);
        return true;
    }

    /// @brief Render checkbox with visual state indicator
    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.checkbox(_state);
    }

private:
    bool _state;
};

}// namespace kf::ui::widgets
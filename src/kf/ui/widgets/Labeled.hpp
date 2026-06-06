// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Labeled.hpp"

#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui::widgets {

struct LabeledTag {};

/// @brief Widget wrapper adding label to another widget
template<typename U> struct Labeled final : LabeledTag, Widget<U>, mixin::Labeled {
    explicit constexpr Labeled(memory::StringView label, Widget<U> &wrapped) noexcept :
        mixin::Labeled{label}, _wrapped{wrapped} {}

    /// @brief Forward click event to wrapped widget
    /// @return Result from wrapped widget's onClick()
    [[nodiscard]] bool onClick() noexcept override { return _wrapped.onClick(); }

    /// @brief Forward change event to wrapped widget
    /// @return Result from wrapped widget's onEventValue()
    [[nodiscard]] bool onEventValue(typename U::EventImpl::Value event_value) noexcept override { return _wrapped.onEventValue(event_value); }

    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.value(this->label());
        render.colon();
        _wrapped.doRender(render);
    }

private:
    Widget<U> &_wrapped;
};

}// namespace kf::ui::widgets
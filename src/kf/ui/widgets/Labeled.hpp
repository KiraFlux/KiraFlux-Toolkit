// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui::widgets {

struct LabeledTag {};

/// @brief Widget wrapper adding label to another widget
/// @tparam W Type of widget being labeled (must inherit from Widget)
template<typename U, typename W> struct Labeled final : LabeledTag, Widget<U> {
    KF_CHECK_IMPL(W, ::kf::ui::widgets::WidgetTag);

    using Wrapped = W;///< Type of wrapped widget implementation

    Wrapped wrapped;///< Wrapped widget instance

    explicit Labeled(typename U::PageImpl &root, memory::StringView label, W impl) :
        Widget<U>{root}, _label{label}, wrapped{std::move(impl)} {}

    /// @brief Forward click event to wrapped widget
    /// @return Result from wrapped widget's onClick()
    [[nodiscard]] bool onClick() noexcept override { return wrapped.onClick(); }

    /// @brief Forward change event to wrapped widget
    /// @return Result from wrapped widget's onEventValue()
    [[nodiscard]] bool onEventValue(typename U::EventImpl::Value event_value) noexcept override { return wrapped.onEventValue(event_value); }

    /// @brief Render label followed by wrapped widget
    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.value(_label);
        render.colon();
        wrapped.doRender(render);
    }

private:
    memory::StringView _label;///< Label text
};

}// namespace kf::ui::widgets
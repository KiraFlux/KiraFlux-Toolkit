// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Range.hpp"
#include "kf/concepts.hpp"
#include "kf/mixin/Configured.hpp"
#include "kf/mixin/ValueCallbacked.hpp"
#include "kf/primitives.hpp"

#include "kf/ui/Decoration.hpp"
#include "kf/ui/Request.hpp"
#include "kf/ui/Style.hpp"
#include "kf/ui/UiTraits.hpp"

namespace kf::internal {

template<typename T> struct SliderConfig final {
    Range<T> value_range;///< value range (slider value will clamped at this value)
    T default_value;     ///< slider value by default
    T step;              ///< slider value adjust step
    bool init_show_value;///< show value by default
};

}// namespace kf::internal

namespace kf::ui::widget {

struct SliderTag {};

/// @tparam U UI Traits Type
/// @tparam T Type of slider Value
template<implements<UiTraitsTag> U, typename T> struct Slider :

    SliderTag,
    U::Widget,
    mixin::ValueCallbacked<T>,
    mixin::Configured<internal::SliderConfig<T>>

{

    using Config = internal::SliderConfig<T>;

    explicit constexpr Slider(const Config &config, Style style = Style::defaults()) noexcept :
        U::Widget{style},
        mixin::ValueCallbacked<T>{config.value_range.clamped(config.default_value)},
        mixin::Configured<Config>{config}, _show_value{config.init_show_value} {}

    /// @brief Toggle slider numeric value display
    Request onClick() noexcept override {
        _show_value = not _show_value;

        return Request::Redraw;
    }

    /// @brief Adjust value
    /// @param event_value adjust change scale
    Request onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        this->value(this->config().value_range.clamped(U::template ArithmeticAdjuster<T>::adjust(this->value(), this->config().step, event_value)));

        return Request::Redraw;
    }

    void doRender(typename U::RendererImpl &render) const noexcept override {
        if (_show_value) {
            render.value(this->value());
            render.decoration(Decoration::Space);
        }

        render.slider(static_cast<kf::f32>(this->value()) / this->config().value_range.length());
    }

private:
    bool _show_value;
};

}// namespace kf::ui::widget
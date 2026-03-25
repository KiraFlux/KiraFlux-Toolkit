
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/memory/Array.hpp"
#include "kf/mixin/Callbacked.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/NonCopyable.hpp"

#include "kf/ui/internal/ComboBoxItem.hpp"
#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui {
namespace internal {
template<typename T, usize N> struct ComboBoxConfig final : mixin::NonCopyable {
    static_assert(N >= 1, "N >= 1");

    using ItemContainer = memory::Array<ComboBoxItem<T>, N>;///< Container type for options

    ItemContainer items;///< Available options
};
}// namespace internal

namespace widgets {
struct ComboBoxTag {};

/// @brief Combo box for selecting from predefined options
/// @tparam T Value type for options
/// @tparam N Number of options (must be >= 1)
template<typename U, typename T, usize N>
struct ComboBox final : ComboBoxTag, Widget<U>, mixin::Callbacked<T>, mixin::Configurable<internal::ComboBoxConfig<T, N>> {
    using Config = internal::ComboBoxConfig<T, N>;

    using mixin::Configurable<Config>::Configurable;

    explicit ComboBox(typename U::PageImpl &page, const Config &config) :
        Widget<U>{page}, mixin::Configurable<Config>{config} {}

    /// @brief Change selection based on direction
    /// @param value Navigation direction (positive/negative)
    [[nodiscard]] bool onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        moveCursor(event_value);
        this->invoke(this->config().items[_cursor].value());
        return true;// redraw required after selection change
    }

    /// @brief Render current selection
    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.beginAltBlock();
        render.value(this->config().items[_cursor].key());
        render.endAltBlock();
    }

private:
    isize _cursor{0};///< Current selection index

    /// @brief Move selection cursor with circular wrapping
    void moveCursor(isize delta) noexcept {
        _cursor = (_cursor + delta + N) % N;
    }
};

}// namespace widgets
}// namespace kf::ui
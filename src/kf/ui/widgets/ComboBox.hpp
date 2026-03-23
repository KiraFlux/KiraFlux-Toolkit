
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/memory/Array.hpp"
#include "kf/mixin/Callbacked.hpp"

#include "kf/ui/internal/ComboBoxItem.hpp"
#include "kf/ui/widgets/Widget.hpp"

namespace kf::ui::widgets {

struct ComboBoxTag {};

/// @brief Combo box for selecting from predefined options
/// @tparam T Value type for options
/// @tparam N Number of options (must be >= 1)
template<typename U, typename T, usize N> struct ComboBox final : ComboBoxTag, Widget<U>, mixin::Callbacked<T> {
    static_assert(N >= 1, "N >= 1");

    using Value = T;                             ///< ComboBox value type
    using Item = internal::ComboBoxItem<Value>;  ///< Item type (in option)
    using ItemContainer = memory::Array<Item, N>;///< Container type for options

    explicit ComboBox(ItemContainer items) noexcept :
        _items{items} {}

    explicit ComboBox(typename U::PageImpl &root, ItemContainer items) :
        Widget<U>{root}, _items{items} {}

    /// @brief Change selection based on direction
    /// @param value Navigation direction (positive/negative)
    [[nodiscard]] bool onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        moveCursor(event_value);
        mixin::Callbacked<Value>::invoke(_items[_cursor].value());
        return true;// redraw required after selection change
    }

    /// @brief Render current selection
    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.beginAltBlock();
        render.value(_items[_cursor].key());
        render.endAltBlock();
    }

private:
    const ItemContainer _items;///< Available options
    isize _cursor{0};          ///< Current selection index

    /// @brief Move selection cursor with circular wrapping
    void moveCursor(isize delta) noexcept {
        _cursor = (_cursor + delta + N) % N;
    }
};

}// namespace kf::ui::widgets
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Slice.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/mixin/Callbacked.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/Labeled.hpp"
#include "kf/mixin/NonCopyable.hpp"

#include "kf/ui/widgets/Widget.hpp"

namespace kf::internal {

template<typename T> struct ComboBoxItem final : mixin::Labeled {

    constexpr ComboBoxItem(memory::StringView label, const T &value) noexcept :
        mixin::Labeled{label}, _value{value} {}

    [[nodiscard]] const T &value() const noexcept { return _value; }

    void value(const T &new_value) noexcept { _value = new_value; }

private:
    T _value;
};

template<> struct ComboBoxItem<memory::StringView> final : mixin::Labeled {

    template<usize N> constexpr ComboBoxItem(const char (&str)[N]) noexcept :// NOLINT(*-explicit-constructor)
        mixin::Labeled{str} {}

    [[nodiscard]] memory::StringView value() const noexcept { return this->label(); }

    void value(memory::StringView new_value) noexcept { this->label(new_value); }
};

template<typename T> struct ComboBoxConfig final {
    using Item = ComboBoxItem<T>;

    Slice<Item> items;///< Available options
};

}// namespace kf::internal

namespace kf::ui::widgets {

struct ComboBoxTag {};

/// @brief Combo box for selecting from predefined options
/// @tparam U UI Traits Type
/// @tparam T Value type for options
template<typename U, typename T> struct ComboBox :

    ComboBoxTag,
    Widget<U>,
    mixin::Callbacked<T>,
    mixin::Configurable<internal::ComboBoxConfig<T>>

{
    using Config = internal::ComboBoxConfig<T>;
    using Item = typename Config::Item;

    using mixin::Configurable<Config>::Configurable;

    /// @brief Change selection based on direction
    /// @param value Navigation direction (positive/negative)
    [[nodiscard]] bool onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        moveCursor(event_value);
        this->invoke(this->config().items[_cursor].value());
        return true;// redraw required after selection change
    }

    void doRender(typename U::RenderImpl &render) const noexcept override {
        render.beginAltBlock();
        if (_cursor < totalItems()) {
            render.value(this->config().items[_cursor].label());
        }
        render.endAltBlock();
    }

private:
    isize _cursor{0};

    /// @brief Move selection cursor with circular wrapping
    void moveCursor(isize delta) noexcept {
        const auto n = totalItems();
        if (n > 0) {
            _cursor = (_cursor + delta + n) % n;
        }
    }

    usize totalItems() const noexcept {
        return this->config().items.size();
    }
};

}// namespace kf::ui::widgets
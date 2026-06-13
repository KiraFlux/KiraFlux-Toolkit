// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Slice.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/mixin/Callbacked.hpp"
#include "kf/mixin/Configurable.hpp"
#include "kf/mixin/Labeled.hpp"
#include "kf/mixin/Styled.hpp"

#include "kf/ui/Block.hpp"
#include "kf/ui/Style.hpp"
#include "kf/ui/UiTraits.hpp"

namespace kf::internal {

template<typename T> struct ComboBoxItem final : mixin::Labeled, mixin::Styled {

    constexpr ComboBoxItem(memory::StringView label, T value, ui::Style style = ui::Style::defaults()) noexcept :
        mixin::Labeled{label}, mixin::Styled{style}, _value{value} {}

    [[nodiscard]] T value() const noexcept {
        return _value;
    }

    void value(T new_value) noexcept {
        _value = new_value;
    }

private:
    T _value;
};

template<> struct ComboBoxItem<memory::StringView> final : mixin::Labeled, mixin::Styled {

    template<usize N> constexpr ComboBoxItem(const char (&str)[N], ui::Style style = ui::Style::defaults()) noexcept :
        mixin::Labeled{str}, mixin::Styled{style} {}

    [[nodiscard]] memory::StringView value() const noexcept {
        return this->label();
    }

    void value(memory::StringView new_value) noexcept {
        this->label(new_value);
    }
};

template<typename T> struct ComboBoxConfig final {
    using Item = ComboBoxItem<T>;

    Slice<Item> items;///< Available options (Always NOT empty)
};

}// namespace kf::internal

namespace kf::ui::widgets {

struct ComboBoxTag {};

/// @brief Combo box for selecting from predefined options
/// @tparam U UI Traits Type
/// @tparam T Value type for options
template<typename U, typename T> struct ComboBox :

    ComboBoxTag,
    U::Widget,
    mixin::Callbacked<T>,
    mixin::Configurable<internal::ComboBoxConfig<T>>

{
    KF_CHECK_IMPL(U, ::kf::ui::UiTraitsTag);

    using Config = internal::ComboBoxConfig<T>;

    explicit constexpr ComboBox(const Config &config, Style style = Style::defaults()) noexcept :
        U::Widget{style}, mixin::Configurable<Config>::Configurable{config} {}

    /// @brief Set selection to the first item whose value equals `new_value`
    /// @param new_value Value to match against items
    /// @note If no matching item is found, the selection remains unchanged
    /// @note undefined behavior if `config().items` is empty
    void value(const T &new_value) noexcept {
        for (auto i = 0u; i < totalItems(); i += 1) {
            if (this->config().items[i].value() == new_value) {
                _cursor = i;
                return;
            }
        }
    }

    /// @return Reference to selected item's value
    /// @note undefined behavior if `config().items` is empty
    const T &value() const noexcept {
        return this->config().items[_cursor].value();
    }

    /// @brief Change selection based on direction
    /// @param value Navigation direction (positive/negative)
    [[nodiscard]] bool onEventValue(typename U::EventImpl::Value event_value) noexcept override {
        moveCursor(event_value);
        this->invoke(this->config().items[_cursor].value());
        return true;// redraw required after selection change
    }

    void doRender(typename U::RenderImpl &render) const noexcept override {
        const auto &item = this->config().items[_cursor];

        render.beginBlock(Block::Alternative);

        render.foreground(item.foreground());
        render.background(item.background());

        render.value(item.label());

        render.foreground(this->foreground());
        render.background(this->background());

        render.endBlock(Block::Alternative);
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
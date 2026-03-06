// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/memory/StringView.hpp"

namespace kf::ui::internal {

template<typename T> struct ComboBoxItem {

private:
    const memory::StringView _key;
    const T _value;

public:
    [[nodiscard]] memory::StringView key() const noexcept { return _key; }

    [[nodiscard]] T value() const noexcept { return _value; }

    constexpr ComboBoxItem(const memory::StringView key, const T value) noexcept :
        _key{key}, _value{value} {}
};

template<> struct ComboBoxItem<memory::StringView> {

private:
    const memory::StringView _key;

public:
    [[nodiscard]] memory::StringView key() const noexcept { return _key; }

    [[nodiscard]] memory::StringView value() const noexcept { return _key; }

    template<usize N> constexpr ComboBoxItem(const char (&str)[N]) noexcept :// NOLINT(*-explicit-constructor)
        _key{str} {}
};

}// namespace kf::ui::internal
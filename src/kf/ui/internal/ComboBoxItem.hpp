// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/memory/StringView.hpp"

namespace kf::ui::internal {

template<typename T> struct ComboBoxItem {

private:
    const memory::StringView key_;
    const T value_;

public:
    [[nodiscard]] memory::StringView key() const noexcept { return key_; }

    [[nodiscard]] T value() const noexcept { return value_; }

    constexpr ComboBoxItem(const memory::StringView key, const T value) noexcept :
        key_{key}, value_{value} {}
};

template<> struct ComboBoxItem<memory::StringView> {

private:
    const memory::StringView key_;

public:
    [[nodiscard]] memory::StringView key() const noexcept { return key_; }

    [[nodiscard]] memory::StringView value() const noexcept { return key_; }

    template<usize N> constexpr ComboBoxItem(const char (&str)[N]) noexcept :// NOLINT(*-explicit-constructor)
        key_{str} {}
};

}// namespace kf::ui::internal
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/attributes.hpp"
#include "kf/memory/StringView.hpp"

namespace kf {// NOLINT(*-concat-nested-namespaces) // for c++11 capability
namespace ui {// NOLINT(*-concat-nested-namespaces)

/// kf::ui detail
namespace detail {// NOLINT(*-concat-nested-namespaces)

template<typename T> struct ComboBoxItem {

private:
    const StringView key_;
    const T value_;

public:
    kf_nodiscard StringView key() const noexcept { return key_; }

    kf_nodiscard T value() const noexcept { return value_; }

    constexpr ComboBoxItem(const StringView key, const T value) noexcept :
        key_{key}, value_{value} {}
};

template<> struct ComboBoxItem<StringView> {

private:
    const StringView key_;

public:
    kf_nodiscard StringView key() const noexcept { return key_; }

    kf_nodiscard StringView value() const noexcept { return key_; }

    template<usize N> constexpr ComboBoxItem(const char (&str)[N]) noexcept :// NOLINT(*-explicit-constructor)
        key_{str} {}
};

}// namespace detail
}// namespace ui
}// namespace kf
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/StringView.hpp"

namespace kf::mixin {

struct LabeledTag {};

/// @brief Adds Label
struct Labeled : LabeledTag {

    explicit constexpr Labeled(StringView label = {}) noexcept :
        _label{label} {}

    [[nodiscard]] constexpr StringView label() const noexcept {
        return _label;
    }

    constexpr void label(StringView new_label) noexcept {
        _label = new_label;
    }

private:
    StringView _label;
};

}// namespace kf::mixin
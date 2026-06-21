// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/memory/StringView.hpp"

namespace kf::mixin {

struct LabeledTag {};

/// @brief Adds Label
struct Labeled : LabeledTag {

    explicit constexpr Labeled(memory::StringView label = {}) noexcept :
        _label{label} {}

    [[nodiscard]] constexpr memory::StringView label() const noexcept {
        return _label;
    }

    void label(memory::StringView new_label) noexcept {
        _label = new_label;
    }

private:
    memory::StringView _label;
};

}// namespace kf::mixin

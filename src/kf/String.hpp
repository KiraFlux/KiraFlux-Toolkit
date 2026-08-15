// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file String.hpp

#pragma once

#include "kf/Stack.hpp"
#include "kf/StringView.hpp"
#include "kf/core.hpp"

namespace kf {

/// @brief Dynamic string builder (Stack<char>) with formatting and C‑string conversion.
struct String : Stack<char> {

    using Stack<char>::Stack;

    /// @brief Make string null-terminated and get data pointer
    [[nodiscard]] constexpr char const *cString() noexcept {
        auto const terminator_index = this->full() ? this->capacity() - 1 : this->length();

        (*this)[terminator_index] = '\0';

        return this->data();
    }

    /// @brief Get StringView
    [[nodiscard]] constexpr StringView view() const noexcept {
        return StringView{this->data(), this->length()};
    }
};

}// namespace kf
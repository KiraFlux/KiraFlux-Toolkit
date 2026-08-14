// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    String.hpp

#pragma once

#include "kf/Array.hpp"
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

    /// @brief Get char array with formatted content
    /// @tparam N Array length
    template<usize N, typename... Args> [[nodiscard]] static constexpr auto formatted(internal::FormatString<Args...> const &fmt, Args const &...args) noexcept -> Array<char, N> {
        Array<char, N> ret{};

        String{ret.slice()}.format(fmt, args...);

        return ret;
    }
};

}// namespace kf
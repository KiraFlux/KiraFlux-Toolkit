// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>
#include <type_traits>

#include "kf/Stack.hpp"
#include "kf/StringView.hpp"
#include "kf/primitives.hpp"

namespace kf::internal {

struct FormatToken {
    enum Kind : u8 {
        Literal,
        Anchor
    } kind;

    usize start;
    usize length;
};

struct FormatParserResult {
    static constexpr auto max_tokens{32u};

    FormatToken tokens[max_tokens];
    usize count;

    constexpr void putAnchor(usize position) noexcept {
        put({
            .kind = FormatToken::Anchor,
            .start = position,
            .length = 2,// "{}"
        });
    }

    constexpr void putLiteral(usize start, usize length) noexcept {
        put({
            .kind = FormatToken::Literal,
            .start = start,
            .length = length,
        });
    }

    constexpr void put(const FormatToken &token) noexcept {
        tokens[count] = token;
        count += 1;
    }
};

struct FormatParser {
    static constexpr auto
        anchor_begin_char{'{'},
        anchor_end_char{'}'};

    template<usize M> static constexpr FormatParserResult parse(const char (&fmt)[M]) {
        constexpr auto format_literal_length{M - 1};

        FormatParserResult result{
            .count = 0,
        };

        auto i = 0u;
        while (i < format_literal_length and result.count < FormatParserResult::max_tokens) {
            //  {{
            if (fmt[i] == anchor_begin_char and (i + 1) < format_literal_length and fmt[i + 1] == anchor_begin_char) {
                result.putLiteral(i, 2);
                i += 2;
                continue;
            }

            //  }}
            if (fmt[i] == anchor_end_char and (i + 1) < format_literal_length and fmt[i + 1] == anchor_end_char) {
                result.putLiteral(i, 2);
                i += 2;
                continue;
            }

            if (fmt[i] == anchor_begin_char) {
                auto start = i;
                i += 1;

                if (i < format_literal_length and fmt[i] == anchor_end_char) {
                    // {} — Anchor
                    result.putAnchor(start);
                    i += 1;
                    continue;
                }

                // Invalid anchor. Skip to next anchor_end_char
                while (i < format_literal_length and fmt[i] != anchor_end_char) {
                    i += 1;
                }

                if (i < format_literal_length) {
                    i += 1;
                }

                result.putLiteral(start, i - start);
            } else {

                // Literal
                auto start = i;
                while (i < format_literal_length and fmt[i] != anchor_begin_char and fmt[i] != anchor_end_char) {
                    i += 1;
                }

                result.putLiteral(start, i - start);
            }
        }
        return result;
    }
};

}// namespace kf::internal

namespace kf {

struct String : Stack<char> {

    using Stack<char>::Stack;

    /// @brief Append single character
    /// @param c Char to add
    constexpr void append(char c) noexcept {
        if (this->full()) {
            return;
        }

        this->push(c);
    }

    /// @brief Append C-String
    /// @param str Should be null-terminated
    constexpr void append(const char *str) noexcept {
        if (nullptr == str) {
            return;
        }

        for (; *str != '\0'; str += 1) {
            if (this->full()) {
                return;
            }

            this->push(*str);
        }
    }

    /// @brief Append Char sequence
    /// @tparam I Implementation class of char sequence
    /// @param char_sequence
    template<typename I> constexpr void append(const I &char_sequence) noexcept {
        for (auto i = 0u; i < char_sequence.length(); i += 1) {
            if (this->full()) {
                return;
            }

            this->push(char_sequence[i]);
        }
    }

    /// @brief Append integer to string
    /// @param value Integer value to append
    constexpr void append(i64 value) noexcept {
        if (value == 0) {
            push('0');
            return;
        }

        const bool is_negative = (value < 0);
        const auto abs_value = is_negative ? static_cast<u64>(-value) : static_cast<u64>(value);

        auto digits = 0u;
        char buffer[21]{};

        for (auto v = abs_value; v > 0; v /= 10) {
            buffer[digits] = static_cast<char>('0' + (v % 10));
            digits += 1;
        }

        if (this->length() + (is_negative ? 1 : 0) + digits > this->capacity()) {
            return;
        }

        if (is_negative) {
            (void) push('-');
        }

        while (digits > 0) {
            digits -= 1;
            (void) push(buffer[digits]);
        }
    }

    /// @brief Append floating-point number to string
    /// @param value Floating-point value
    /// @param precision Number of decimal places to show
    constexpr void append(f64 value, usize precision = 3) noexcept {
        if (std::isnan(value)) {
            append("nan");
            return;
        }

        const bool is_negative = value < 0;

        if (std::isinf(value)) {
            append(is_negative ? "-inf" : "+inf");
            return;
        }

        if (is_negative) {
            value = -value;
        }

        const auto integer_part = static_cast<i64>(value);
        const auto fraction_part = value - integer_part;

        auto integer_digits = 0u;
        for (auto v = integer_part; v > 0; v /= 10) {
            integer_digits += 1;
        }
        if (integer_digits == 0) {
            integer_digits = 1;
        }

        const auto chars_needed = (is_negative ? 1 : 0) + integer_digits + (precision == 0 ? 0 : 1 + precision);
        if (this->length() + chars_needed > this->capacity()) {
            return;
        }

        if (is_negative) {
            (void) push('-');
        }

        (void) append(integer_part);

        if (precision == 0) {
            return;
        }

        (void) push('.');

        auto fraction = fraction_part;
        for (auto i = 0u; i < precision; i += 1) {
            fraction *= 10.0;

            const auto fraction_digit = static_cast<u8>(fraction);
            (void) push('0' + fraction_digit);

            fraction -= fraction_digit;

            if (fraction < 1e-12) {
                break;
            }
        }
    }

    /// @brief Append boolean value
    constexpr void append(bool value) noexcept {
        append(value ? "true" : "false");
    }

    /// @brief Make string null-terminated and get data pointer
    [[nodiscard]] constexpr const char *cString() noexcept {
        const auto terminator_index = this->full() ? this->capacity() - 1 : this->length();

        (*this)[terminator_index] = '\0';

        return this->data();
    }

    /// @brief Get StringView
    [[nodiscard]] constexpr StringView view() const noexcept {
        return StringView(this->data(), this->length());
    }

    /// @brief Format into string
    /// @tparam ...Args argument types (auto-deduced), used for `append` method static dispatching
    /// @tparam format_literal_length format string literal length (auto-deduced)
    /// @param fmt format string literal
    /// @param ...args format arguments
    /// @note For argument placement use `{}` as anchor
    template<usize format_literal_length, typename... Args> constexpr void format(const char (&fmt)[format_literal_length], const Args &...args) {
        constexpr auto parser_result = internal::FormatParser::parse(fmt);

        static_assert(
            parser_result.count == sizeof...(Args),
            "Number of {} placeholders does not match number of arguments");

        this->reset();

        if constexpr (parser_result.count == 0) {
            this->append(StringView(fmt, format_literal_length - 1));
        } else {
            formatImpl<0, 0>(fmt, parser_result, args...);
        }
    }

private:
    template<usize token_index, usize arg_index, typename First, typename... Rest> constexpr void formatImpl(

        const char *fmt,
        const internal::FormatParserResult &parser_result,
        const First &first,
        const Rest &...rest

    ) {
        const auto &token = parser_result.tokens[token_index];

        if (token.kind == internal::FormatToken::Literal) {
            this->append(StringView(fmt + token.start, token.length));
        } else {
            // Anchor
            this->append(first);
        }

        if constexpr (token_index + 1 < parser_result.count) {
            constexpr auto next_token_index = token_index + 1;
            constexpr auto next_arg_index = arg_index + ((token.kind == internal::FormatToken::Anchor) ? 1 : 0);

            if constexpr (token.kind == internal::FormatToken::Anchor) {
                formatImpl<next_token_index, next_arg_index>(fmt, parser_result, rest...);
            } else {
                formatImpl<next_token_index, next_arg_index>(fmt, parser_result, first, rest...);
            }
        }
    }
};

}// namespace kf
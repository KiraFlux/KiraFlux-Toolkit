// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <tuple>
#include <type_traits>

#include "kf/Array.hpp"
#include "kf/Stack.hpp"
#include "kf/StringView.hpp"
#include "kf/math.hpp"
#include "kf/mixin/Representable.hpp"
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

struct FormatResult {
    static constexpr auto max_tokens{32u};

    usize anchor_indices[max_tokens];
    FormatToken tokens[max_tokens];
    usize count;

    constexpr void putAnchor(usize position, usize arg_index) noexcept {
        anchor_indices[count] = arg_index;
        tokens[count] = {
            .kind = FormatToken::Anchor,
            .start = position,
            .length = 2,// "{}"
        };

        count += 1;
    }

    constexpr void putLiteral(usize start, usize length) noexcept {
        tokens[count] = {
            .kind = FormatToken::Literal,
            .start = start,
            .length = length,
        };

        count += 1;
    }
};

template<typename... Args> struct BasicFormatString {

    static constexpr auto
        anchor_begin_char{'{'},
        anchor_end_char{'}'};

    const char *str;
    usize length;
    internal::FormatResult result;

    template<usize N> consteval BasicFormatString(const char (&s)[N]) noexcept :
        str{s}, length{N - 1}, result{parse(s)} {

        if (result.count != sizeof...(Args)) {

            //TODO: compile-time termination
        }
    }

    template<usize M> static constexpr FormatResult parse(const char (&fmt)[M]) noexcept {
        constexpr auto format_literal_length{M - 1};

        FormatResult result{
            .count = 0,
        };

        auto i = 0u;
        auto arg_index = 0u;

        while (i < format_literal_length and result.count < FormatResult::max_tokens) {
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
                    result.putAnchor(start, arg_index);
                    arg_index += 1;
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

template<typename... Args> using FormatString = BasicFormatString<std::type_identity_t<Args>...>;

template<typename T> static constexpr bool is_c_string_v{

    std::is_same_v<T, char *> or
    std::is_same_v<T, const char *> or
    (std::is_array_v<T> and std::is_same_v<std::remove_extent_t<T>, char>)

};

template<typename> static constexpr bool always_false_v{false};

}// namespace kf::internal

namespace kf {

struct String : Stack<char> {

    using Stack<char>::Stack;

    /// @brief Append string representation of value
    /// @param value Value to represent
    /// @param precision Precision for float-point number
    constexpr void append(auto const &value, usize precision = 3) noexcept {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, char>) {

            (void) this->push(value);

        } else if constexpr (std::is_same_v<T, bool>) {

            appendNullTerminatedString(value ? "true" : "false");

        } else if constexpr (internal::is_c_string_v<T>) {

            appendNullTerminatedString(static_cast<const char *>(value));

        } else if constexpr (std::is_base_of_v<SequenceTag, T>) {

            for (auto c: value) {
                if (this->full()) {
                    break;
                }

                (void) this->push(c);
            }

        } else if constexpr (std::is_base_of_v<mixin::RepresentableTag, T>) {

            append(value.repr());

        } else if constexpr (std::is_integral_v<T>) {

            appendInteger(value);

        } else if constexpr (std::is_floating_point_v<T>) {

            appendReal(value, precision);

        } else {
            static_assert(internal::always_false_v<T>, "Unsupported type for append");
        }
    }

    /// @brief Make string null-terminated and get data pointer
    [[nodiscard]] constexpr const char *cString() noexcept {
        const auto terminator_index = this->full() ? this->capacity() - 1 : this->length();

        (*this)[terminator_index] = '\0';

        return this->data();
    }

    /// @brief Get StringView
    [[nodiscard]] constexpr StringView view() const noexcept {
        return StringView{this->data(), this->length()};
    }

    /// @brief Format into string
    /// @tparam ...Args argument types (auto-deduced), used for `append` method static dispatching
    /// @param fmt format string implicit consteval-constructed from literal
    /// @param ...args format arguments
    /// @note For argument placement use `{}` as anchor
    template<typename... Args> constexpr void format(internal::FormatString<Args...> fmt, const Args &...args) noexcept {
        this->reset();

        const auto tuple = std::forward_as_tuple(args...);

        for (auto i = 0u; i < fmt.result.count; i += 1) {
            const auto &token = fmt.result.tokens[i];

            if (token.kind == internal::FormatToken::Literal) {
                this->append(StringView(fmt.str + token.start, token.length));
            } else {

                if constexpr (sizeof...(Args) > 0) {
                    applyArg<0>(fmt.result.anchor_indices[i], tuple);
                }
            }
        }
    }

    /// @brief Get char array with formatted content
    /// @tparam N Array length
    template<usize N, typename... Args> [[nodiscard]] static constexpr auto formatted(internal::FormatString<Args...> fmt, const Args &...args) noexcept -> Array<char, N> {
        Array<char, N> ret{};

        String{ret.slice()}.format(fmt, args...);

        return ret;
    }

private:
    constexpr void appendNullTerminatedString(const char *str) noexcept {
        if (nullptr == str) {
            return;
        }

        while (*str != '\0') {
            if (this->full()) {
                break;
            }

            (void) this->push(*str);
            str += 1;
        }
    }

    constexpr void appendInteger(i64 value) noexcept {
        if (0 == value) {
            (void) push('0');
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

        if (this->length() + static_cast<usize>(is_negative) + digits > this->capacity()) {
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

    constexpr void appendReal(f64 value, usize precision) noexcept {
        if (math::isnan(value)) {
            appendNullTerminatedString("nan");
            return;
        }

        const bool is_negative = (value < 0);
        const bool just_integer_part = (0 == precision);

        if (math::isinf(value)) {
            appendNullTerminatedString(is_negative ? "-inf" : "+inf");
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

        const auto chars_needed = static_cast<usize>(is_negative) + integer_digits + static_cast<usize>(just_integer_part) + precision;
        if (this->length() + chars_needed > this->capacity()) {
            return;
        }

        if (is_negative) {
            (void) push('-');
        }

        appendInteger(integer_part);

        if (just_integer_part) {
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

    template<usize I, typename... Args> constexpr void applyArg(usize index, const std::tuple<const Args &...> &tuple) {
        if (I == index) {

            append(std::get<I>(tuple));

        } else {

            if constexpr (I + 1 < sizeof...(Args)) {
                applyArg<I + 1>(index, tuple);
            }
        }
    }
};

}// namespace kf
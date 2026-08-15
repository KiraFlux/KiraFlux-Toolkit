// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/Writable.hpp

#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "kf/StringView.hpp"
#include "kf/core.hpp"
#include "kf/math.hpp"

#include "kf/mixin/ReprTo.hpp"
#include "kf/mixin/Representable.hpp"

namespace kf::mixin {

template<typename T> struct WritableTag {
    struct WriteTraits {
        using Type = T;
    };
};

}// namespace kf::mixin

namespace kf::internal {

template<typename Impl, typename T> struct WritableBase : mixin::WritableTag<T> {

    /// @brief Write Value
    /// @param value Value to write (perfect-forwarded)
    /// @return `true` if write successful, `false` otherwise
    [[nodiscard]] constexpr bool write(auto &&value) noexcept {
        return static_cast<Impl *>(this)->writeImpl(std::forward<decltype(value)>(value));
    }
};

struct FormatToken {
    u8 start, length;

    enum Kind : u8 {
        Literal,
        Anchor
    } kind;
};

struct FormatResult {
    static constexpr auto max_tokens{16u};

    FormatToken tokens[max_tokens];
    u8 anchor_indices[max_tokens];
    u8 count;

    constexpr void putAnchor(u8 position, u8 arg_index) noexcept {
        anchor_indices[count] = arg_index;
        tokens[count] = {
            .start = position,
            .length = 2,// "{}"
            .kind = FormatToken::Anchor,
        };

        count += 1;
    }

    constexpr void putLiteral(u8 start, u8 length) noexcept {
        tokens[count] = {
            .start = start,
            .length = length,
            .kind = FormatToken::Literal,
        };

        count += 1;
    }
};

template<typename... Args> struct BasicFormatString {

    static constexpr auto
        anchor_begin_char{'{'},
        anchor_end_char{'}'};

    char const *str;
    usize length;
    internal::FormatResult result;

    template<usize N> consteval BasicFormatString(char const (&s)[N]) noexcept :
        str{s}, length{N - 1}, result{parse(s)} {

        if (result.count != sizeof...(Args)) {

            //TODO: compile-time termination
        }
    }

    template<usize M> static constexpr FormatResult parse(char const (&fmt)[M]) noexcept {
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

}// namespace kf::internal

namespace kf::mixin {

/// @brief Add `write(auto &&) -> bool`
/// @tparam Impl Writable mixin implementation class
/// @tparam T Value type
template<typename Impl, typename T> struct Writable;

template<typename Impl, typename T> struct Writable : internal::WritableBase<Impl, T> {};

template<typename Impl> struct Writable<Impl, char> : internal::WritableBase<Impl, char> {

    // static dispatching

    [[nodiscard]] constexpr usize append(char value) noexcept {
        return static_cast<usize>(this->write(value));
    }

    [[nodiscard]] constexpr usize append(bool value) noexcept {
        return appendNullTerminatedString(value ? "true" : "false");
    }

    [[nodiscard]] constexpr usize append(c_string auto const &value) noexcept {
        return appendNullTerminatedString(static_cast<char const *>(value));
    }

    [[nodiscard]] constexpr usize append(integer_type auto value) noexcept {
        return appendInteger(value);
    }

    [[nodiscard]] constexpr usize append(float_type auto value, usize precision = 3) noexcept {
        return appendReal(value, precision);
    }

    [[nodiscard]] constexpr usize append(enum_type auto value) noexcept {
        return (
            appendNullTerminatedString("enum(") +
            appendInteger(static_cast<usize>(value)) +
            append(')'));
    }

    [[nodiscard]] constexpr usize append(implements<RepresentableTag> auto const &value) noexcept {
        return append(value.repr());
    }

    [[nodiscard]] constexpr usize append(implements<ReprToTag> auto const &value) noexcept {
        return value.reprTo(*this);
    }

    // concrete append algorithms

    [[nodiscard]] constexpr usize appendNullTerminatedString(char const *str, usize max_length = static_cast<usize>(-1)) noexcept {
        usize write_count = 0;

        if (nullptr != str) {
            while (write_count < max_length) {
                char const c = str[write_count];

                if (c == '\0') {
                    break;
                }

                if (not this->append(c)) {
                    break;
                }

                write_count += 1;
            }
        }

        return write_count;
    }

    [[nodiscard]] constexpr usize appendInteger(i64 value) noexcept {
        if (0 == value) {
            return this->append('0');
        }

        bool const is_negative = (value < 0);
        auto const abs_value = is_negative ? static_cast<u64>(-value) : static_cast<u64>(value);

        auto digits = 0u;
        char buffer[21]{};

        for (auto v = abs_value; v > 0; v /= 10) {
            buffer[digits] = static_cast<char>('0' + (v % 10));
            digits += 1;
        }

        usize write_count = 0;

        if (is_negative) {
            write_count += this->append('-');
        }

        while (digits > 0) {
            digits -= 1;
            write_count += this->append(buffer[digits]);
        }

        return write_count;
    }

    [[nodiscard]] constexpr usize appendReal(f64 value, usize precision) noexcept {
        if (math::isnan(value)) {
            return appendNullTerminatedString("nan");
        }

        bool const is_negative = (value < 0);
        bool const just_integer_part = (0 == precision);

        if (math::isinf(value)) {
            return appendNullTerminatedString(is_negative ? "-inf" : "+inf");
        }

        if (is_negative) {
            value = -value;
        }

        auto const integer_part = static_cast<i64>(value);
        auto const fraction_part = value - integer_part;

        auto integer_digits = 0u;
        for (auto v = integer_part; v > 0; v /= 10) {
            integer_digits += 1;
        }
        if (integer_digits == 0) {
            integer_digits = 1;
        }

        usize write_count = 0;

        if (is_negative) {
            write_count += this->append('-');
        }

        write_count += appendInteger(integer_part);

        if (just_integer_part) {
            return write_count;
        }

        write_count += this->append('.');

        auto fraction = fraction_part;
        for (auto i = 0u; i < precision; i += 1) {
            fraction *= 10.0;

            auto const fraction_digit = static_cast<u8>(fraction);
            write_count += this->append('0' + fraction_digit);

            fraction -= fraction_digit;

            if (fraction < 1e-12) {
                break;
            }
        }
        return write_count;
    }

    /// @brief Append Formatted string
    /// @tparam ...Args argument types (auto-deduced), used for `append` method static dispatching
    /// @param fmt format string implicit consteval-constructed from literal
    /// @param ...args format arguments
    /// @note For argument placement use `{}` as anchor
    template<typename... Args> [[nodiscard]] constexpr usize appendFormat(internal::FormatString<Args...> const &fmt, Args const &...args) noexcept {
        auto const tuple = std::forward_as_tuple(args...);
        usize write_count = 0;

        for (auto i = 0u; i < fmt.result.count; i += 1) {
            auto const &token = fmt.result.tokens[i];

            if (token.kind == internal::FormatToken::Literal) {
                write_count += this->append(StringView{fmt.str + token.start, token.length});
            } else if constexpr (sizeof...(Args) > 0) {
                write_count += applyArg<0>(fmt.result.anchor_indices[i], tuple);
            }
        }

        return write_count;
    }

private:
    template<usize I, typename... Args> [[nodiscard]] constexpr usize applyArg(usize index, std::tuple<Args const &...> const &tuple) {
        if (I == index) {
            return append(std::get<I>(tuple));
        } else if constexpr (I + 1 < sizeof...(Args)) {
            return applyArg<I + 1>(index, tuple);
        }
        return 0;
    }
};

}// namespace kf::mixin

#define KF_IMPL_WRITABLE(...) friend struct ::kf::internal::WritableBase<__VA_ARGS__>

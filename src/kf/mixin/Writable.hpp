// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <tuple>
#include <type_traits>
#include <utility>

#include "kf/StringView.hpp"
#include "kf/math.hpp"
#include "kf/mixin/Representable.hpp"

namespace kf::mixin {

struct WritableTag {};

}// namespace kf::mixin

namespace kf::internal {

template<typename Impl, typename T> struct WritableBase : mixin::WritableTag {

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

namespace kf::mixin {

/// @brief Add `write(auto &&) -> bool`
/// @tparam Impl Writable mixin implementation class
/// @tparam T Value type
template<typename Impl, typename T> struct Writable;

template<typename Impl, typename T> struct Writable : internal::WritableBase<Impl, T> {};

template<typename Impl> struct Writable<Impl, char> : internal::WritableBase<Impl, char> {

    /// @brief Append string representation of value
    /// @param value Value to represent
    /// @param precision Precision for float-point number
    constexpr void append(auto const &value, usize precision = 3) noexcept {
        using T = std::decay_t<decltype(value)>;

        if constexpr (std::is_same_v<T, char>) {

            (void) this->write(value);

        } else if constexpr (std::is_same_v<T, bool>) {

            appendNullTerminatedString(value ? "true" : "false");

        } else if constexpr (internal::is_c_string_v<T>) {

            appendNullTerminatedString(static_cast<const char *>(value));

        } else if constexpr (std::is_base_of_v<SequenceTag, T>) {

            for (auto c: value) {
                if (not this->write(c)) {
                    break;
                }
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

    /// @brief Format into string
    /// @tparam ...Args argument types (auto-deduced), used for `append` method static dispatching
    /// @param fmt format string implicit consteval-constructed from literal
    /// @param ...args format arguments
    /// @note For argument placement use `{}` as anchor
    template<typename... Args> constexpr void format(const internal::FormatString<Args...> &fmt, const Args &...args) noexcept {
        const auto tuple = std::forward_as_tuple(args...);

        for (auto i = 0u; i < fmt.result.count; i += 1) {
            const auto &token = fmt.result.tokens[i];

            if (token.kind == internal::FormatToken::Literal) {
                this->append(StringView{fmt.str + token.start, token.length});
            } else if constexpr (sizeof...(Args) > 0) {
                applyArg<0>(fmt.result.anchor_indices[i], tuple);
            }
        }
    }

private:
    constexpr void appendNullTerminatedString(const char *str) noexcept {
        if (nullptr == str) {
            return;
        }

        while (*str != '\0') {
            if (not this->write(*str)) {
                break;
            }

            str += 1;
        }
    }

    constexpr void appendInteger(i64 value) noexcept {
        if (0 == value) {
            (void) this->write('0');
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

        if (is_negative) {
            (void) this->write('-');
        }

        while (digits > 0) {
            digits -= 1;
            if (not this->write(buffer[digits])) {
                break;
            }
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

        if (is_negative) {
            (void) this->write('-');
        }

        appendInteger(integer_part);

        if (just_integer_part) {
            return;
        }

        (void) this->write('.');

        auto fraction = fraction_part;
        for (auto i = 0u; i < precision; i += 1) {
            fraction *= 10.0;

            const auto fraction_digit = static_cast<u8>(fraction);
            (void) this->write('0' + fraction_digit);

            fraction -= fraction_digit;

            if (fraction < 1e-12) {
                break;
            }
        }
    }

    template<usize I, typename... Args> constexpr void applyArg(usize index, const std::tuple<const Args &...> &tuple) {
        if (I == index) {
            append(std::get<I>(tuple));
        } else if constexpr (I + 1 < sizeof...(Args)) {
            applyArg<I + 1>(index, tuple);
        }
    }
};

}// namespace kf::mixin

#define KF_IMPL_WRITABLE(...) friend struct ::kf::internal::WritableBase<__VA_ARGS__>

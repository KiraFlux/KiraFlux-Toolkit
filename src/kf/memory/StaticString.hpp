// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cmath>
#include <cstdarg>
#include <cstdio>

#include "kf/Result.hpp"
#include "kf/Slice.hpp"
#include "kf/algorithm.hpp"
#include "kf/memory/Array.hpp"
#include "kf/memory/StringView.hpp"

namespace kf::memory {

/// @brief Fixed-size string buffer with compile-time capacity
/// @tparam N Maximum string capacity (excluding null terminator)
/// @note Always null-terminated, safe for C APIs
template<usize N> struct StaticString {
    static_assert(N > 0, "StaticString capacity must be positive");

    enum class Error {
        Truncated,
        FormatFailed
    };

private:
    Array<char, N + 1> _buffer;// +1 for null terminator
    usize _size{0};            // Current length (excluding null terminator)

public:
    /// @brief Default constructor (empty string)
    constexpr StaticString() noexcept {
        _buffer[0] = '\0';
    }

    /// @brief Construct from string literal
    /// @tparam M Literal size (including null terminator)
    template<usize M> constexpr StaticString(const char (&str)[M]) noexcept {
        static_assert(M > 0, "String literal must not be empty");
        assign(StringView(str, min(M - 1, N)));
    }

    template<usize M, typename... Args> [[nodiscard]] static constexpr StaticString formatted(const char (&fmt)[M], Args... args) noexcept {
        static_assert(M > 0, "String literal must not be empty");
        StaticString ret{};
        (void) ret.format(fmt, args...);
        return ret;
    }

    /// @brief Construct from StringView
    explicit constexpr StaticString(StringView view) noexcept {
        assign(view);
    }

    /// @brief Construct from C-string
    constexpr StaticString(const char *str) noexcept {
        assign(StringView(str));
    }

    /// @brief Get string data pointer
    [[nodiscard]] constexpr char *data() noexcept {
        return _buffer.data();
    }

    /// @brief Get const string data pointer
    [[nodiscard]] constexpr const char *data() const noexcept {
        return _buffer.data();
    }

    /// @brief Get string as StringView
    [[nodiscard]] constexpr StringView view() const noexcept {
        return StringView(_buffer.data(), _size);
    }

    /// @brief Get string as Slice
    [[nodiscard]] constexpr Slice<const char> slice() const noexcept {
        return Slice<const char>{_buffer.data(), _size};
    }

    /// @brief Get mutable Slice (use with caution)
    [[nodiscard]] constexpr Slice<char> slice() noexcept {
        return Slice<char>{_buffer.data(), _size};
    }

    /// @brief Get current string length
    [[nodiscard]] constexpr usize size() const noexcept {
        return _size;
    }

    /// @brief Get maximum capacity (excluding null terminator)
    [[nodiscard]] constexpr usize capacity() const noexcept {
        return N;
    }

    /// @brief Check if string is empty
    [[nodiscard]] constexpr bool empty() const noexcept {
        return _size == 0;
    }

    /// @brief Check if string is full (no more characters can be added)
    [[nodiscard]] constexpr bool full() const noexcept {
        return _size == N;
    }

    /// @brief Clear string contents
    constexpr void clear() noexcept {
        _size = 0;
        _buffer[0] = '\0';
    }

    /// @brief Assign string from StringView
    constexpr StaticString &assign(StringView view) noexcept {
        _size = min(view.size(), N);
        for (usize i = 0; i < _size; ++i) {
            _buffer[i] = view[i];
        }
        _buffer[_size] = '\0';
        return *this;
    }

    /// @brief Append character
    /// @param ch Character to append
    /// @return true if character was appended, false if buffer full
    [[nodiscard]] constexpr bool push(char ch) noexcept {
        if (_size >= N) { return false; }
        _buffer[_size] = ch;
        ++_size;
        _buffer[_size] = '\0';
        return true;
    }

    /// @brief Remove last character
    /// @return true if character was removed, false if string empty
    [[nodiscard]] constexpr bool pop() noexcept {
        if (_size == 0) { return false; }
        --_size;
        _buffer[_size] = '\0';
        return true;
    }

    /// @brief Append StringView
    /// @param view String to append
    /// @return Number of characters actually appended
    [[nodiscard]] constexpr usize append(StringView view) noexcept {
        const usize available = N - _size;
        const usize to_append = min(view.size(), available);

        for (usize i = 0; i < to_append; ++i) {
            _buffer[_size + i] = view[i];
        }

        _size += to_append;
        _buffer[_size] = '\0';
        return to_append;
    }

    /// @brief Append C-string
    /// @param str C-string to append
    /// @return Number of characters actually appended
    [[nodiscard]] constexpr usize append(const char *str) noexcept {
        return append(StringView(str));
    }

    /// @brief Append integer to string
    /// @param value Integer value to append
    /// @return Number of characters appended
    [[nodiscard]] usize append(i32 value) noexcept {
        usize start = _size;
        if (value == 0) { return push('0') ? 1 : 0; }

        bool negative = value < 0;
        u32 abs_val = negative ? -static_cast<u32>(value) : static_cast<u32>(value);

        int digits = 0;
        for (u32 v = abs_val; v; v /= 10) {
            digits += 1;
        }

        if (_size + (negative ? 1 : 0) + digits > N) { return 0; }

        if (negative) {
            (void) push('-');
        }

        char buf[12];
        int i = 0;

        for (u32 v = abs_val; v; v /= 10) {
            buf[i++] = char('0' + (v % 10));
        }

        while (i--) {
            (void) push(buf[i]);
        }

        return _size - start;
    }

    /// @brief Append floating-point number to string
    /// @param value Floating-point value
    /// @param decimal_places Number of decimal places to show
    /// @return Number of characters appended
    [[nodiscard]] usize append(f64 value, u8 prec) noexcept {
        usize start = _size;

        if (std::isnan(value)) { return append("nan"); }

        if (std::isinf(value)) { return append(value > 0 ? "inf" : "-inf"); }

        bool negative = value < 0;
        if (negative) value = -value;

        i32 int_part = static_cast<i32>(value);
        f64 fraction = value - int_part;

        int int_digits = 0;
        for (u32 v = static_cast<u32>(int_part); v; v /= 10) { int_digits += 1; }
        if (int_digits == 0) { int_digits = 1; }

        usize needed = (negative ? 1 : 0) + int_digits;

        if (prec > 0) {
            needed += 1 + prec;
        }

        if (_size + needed > N) { return 0; }

        if (negative) {
            (void) push('-');
        }

        (void) append(int_part);

        if (prec > 0) {
            (void) push('.');
            f64 frac = fraction;

            for (u8 i = 0; i < prec; ++i) {
                frac *= 10.0;
                u8 d = static_cast<u8>(frac);
                (void) push('0' + d);
                frac -= d;

                if (frac < 1e-12) { break; }
            }
        }
        return _size - start;
    }

    /// @brief Insert string at position
    /// @param pos Position to insert at (0 <= pos <= size())
    /// @param view String to insert
    /// @return Number of characters inserted
    [[nodiscard]] constexpr usize insert(usize pos, StringView view) noexcept {
        if (pos > _size) { pos = _size; }

        const usize available = N - _size;
        const usize to_insert = min(view.size(), available);

        // Make space for inserted characters
        for (usize i = _size; i > pos; --i) {
            _buffer[i + to_insert - 1] = _buffer[i - 1];
        }

        // Insert new characters
        for (usize i = 0; i < to_insert; ++i) {
            _buffer[pos + i] = view[i];
        }

        _size += to_insert;
        _buffer[_size] = '\0';
        return to_insert;
    }

    /// @brief Erase characters from string
    /// @param pos Starting position
    /// @param count Number of characters to erase
    /// @return Number of characters erased
    [[nodiscard]] constexpr usize erase(usize pos, usize count = 1) noexcept {
        if (pos >= _size) { return 0; }

        const usize remaining = _size - pos;
        const usize to_erase = min(count, remaining);

        // Shift characters left
        for (usize i = pos; i < _size - to_erase; ++i) {
            _buffer[i] = _buffer[i + to_erase];
        }

        _size -= to_erase;
        _buffer[_size] = '\0';
        return to_erase;
    }

    /// @brief Format string using printf-style syntax
    /// @note Always null-terminates the result
    [[nodiscard]] auto format(const char *format, ...) noexcept -> Result<usize, Error> {
        va_list args;
        va_start(args, format);
        const int result = vsnprintf(_buffer.data(), N + 1, format, args);
        va_end(args);

        if (result < 0) {
            _size = 0;
            _buffer[0] = '\0';
            return error(Error::FormatFailed);
        }

        _size = min(static_cast<usize>(result), N);
        _buffer[_size] = '\0';

        if (static_cast<usize>(result) > N) {
            return error(Error::Truncated);
        }
        return ok(_size);
    }

    /// @brief Trim whitespace from beginning
    /// @return Reference to this string
    constexpr StaticString &trimStart() noexcept {
        usize i = 0;
        while (i < _size and isWhitespace(_buffer[i])) {
            ++i;
        }

        if (i > 0) {
            // Shift characters left
            for (usize j = 0; j < _size - i; ++j) {
                _buffer[j] = _buffer[j + i];
            }
            _size -= i;
            _buffer[_size] = '\0';
        }

        return *this;
    }

    /// @brief Trim whitespace from end
    /// @return Reference to this string
    constexpr StaticString &trimEnd() noexcept {
        while (_size > 0 and isWhitespace(_buffer[_size - 1])) {
            --_size;
        }
        _buffer[_size] = '\0';
        return *this;
    }

    /// @brief Trim whitespace from both ends
    /// @return Reference to this string
    constexpr StaticString &trim() noexcept {
        return trimStart().trimEnd();
    }

    /// @brief Find character in string
    /// @param ch Character to find
    /// @param pos Starting position
    /// @return Option containing position of character if found
    [[nodiscard]] constexpr auto find(char ch, usize pos = 0) const noexcept -> Option<usize> {
        for (usize i = pos; i < _size; ++i) {
            if (_buffer[i] == ch) {
                return some(i);
            }
        }
        return none;
    }

    /// @brief Find substring in string
    /// @param str Substring to find
    /// @param pos Starting position
    /// @return Option containing position of substring if found
    [[nodiscard]] constexpr auto find(StringView str, usize pos = 0) const noexcept -> Option<usize> {
        if (str.size() > _size or pos > _size - str.size()) {
            return none;
        }

        for (usize i = pos; i <= _size - str.size(); ++i) {
            bool found = true;
            for (usize j = 0; j < str.size(); ++j) {
                if (_buffer[i + j] != str[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return some(i);
            }
        }
        return none;
    }

    /// @brief Check if string starts with prefix
    /// @param prefix Prefix to check
    /// @return true if string starts with prefix
    [[nodiscard]] constexpr bool startsWith(StringView prefix) const noexcept {
        return view().startsWith(prefix);
    }

    /// @brief Check if string ends with suffix
    /// @param suffix Suffix to check
    /// @return true if string ends with suffix
    [[nodiscard]] constexpr bool endsWith(StringView suffix) const noexcept {
        return view().endsWith(suffix);
    }

    /// @brief Get character at index (no bounds checking)
    [[nodiscard]] constexpr char operator[](usize index) const noexcept {
        return _buffer[index];
    }

    /// @brief Get mutable character at index (no bounds checking)
    [[nodiscard]] constexpr char &operator[](usize index) noexcept {
        return _buffer[index];
    }

    /// @brief Assignment from StringView
    constexpr StaticString &operator=(StringView view) noexcept {
        return assign(view);
    }

    /// @brief Assignment from C-string
    constexpr StaticString &operator=(const char *str) noexcept {
        return assign(StringView(str));
    }

    /// @brief Assignment from string literal
    template<usize M> constexpr StaticString &operator=(const char (&str)[M]) noexcept {
        return assign(StringView(str, min(M - 1, N)));
    }

    /// @brief Compare with StringView
    [[nodiscard]] constexpr int compare(StringView other) const noexcept {
        return view().compare(other);
    }

    /// @brief Compare with C-string
    [[nodiscard]] constexpr int compare(const char *str) const noexcept {
        return view().compare(StringView(str));
    }

    /// @brief Implicit conversion to StringView
    [[nodiscard]] constexpr operator StringView() const noexcept {
        return view();
    }

    /// @brief Implicit conversion to const char*
    [[nodiscard]] explicit constexpr operator const char *() const noexcept {
        return data();
    }

private:
    /// @brief Check if character is whitespace
    static constexpr bool isWhitespace(char ch) noexcept {
        return ch == ' ' or ch == '\t' or ch == '\n' or ch == '\r' or ch == '\v' or ch == '\f';
    }
};

template<usize N> constexpr bool operator==(const StaticString<N> &lhs, const StaticString<N> &rhs) noexcept {
    return lhs.view() == rhs.view();
}

template<usize N> constexpr bool operator!=(const StaticString<N> &lhs, const StaticString<N> &rhs) noexcept {
    return !(lhs == rhs);
}

/// @brief Compare FixedString with StringView for equality
template<usize N> constexpr bool operator==(const StaticString<N> &lhs, StringView rhs) noexcept {
    return lhs.view() == rhs;
}

/// @brief Compare StringView with FixedString for equality
template<usize N> constexpr bool operator==(StringView lhs, const StaticString<N> &rhs) noexcept {
    return lhs == rhs.view();
}

/// @brief Compare FixedString with C-string for equality
template<usize N> constexpr bool operator==(const StaticString<N> &lhs, const char *rhs) noexcept {
    return lhs.view() == StringView(rhs);
}

/// @brief Compare C-string with FixedString for equality
template<usize N> constexpr bool operator==(const char *lhs, const StaticString<N> &rhs) noexcept {
    return StringView(lhs) == rhs.view();
}

/// @brief Inequality operators (implement via equality)
template<usize N, typename T> constexpr bool operator!=(const StaticString<N> &lhs, const T &rhs) noexcept {
    return !(lhs == rhs);
}

template<usize N, typename T> constexpr bool operator!=(const T &lhs, const StaticString<N> &rhs) noexcept {
    return !(lhs == rhs);
}

}// namespace kf::memory
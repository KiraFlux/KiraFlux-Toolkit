// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdarg>

#include "kf/memory/Array.hpp"
#include "kf/memory/Slice.hpp"
#include "kf/memory/StringView.hpp"
#include "kf/core/attributes.hpp"
#include "kf/algorithm.hpp"


namespace kf {

/// @brief Fixed-size string buffer with compile-time capacity
/// @tparam N Maximum string capacity (excluding null terminator)
/// @note Always null-terminated, safe for C APIs
template<usize N> class ArrayString {
    static_assert(N > 0, "ArrayString capacity must be positive");

private:
    Array<char, N + 1> buffer_;  // +1 for null terminator
    usize size_{0};              // Current length (excluding null terminator)

public:
    /// @brief Default constructor (empty string)
    constexpr ArrayString() noexcept {
        buffer_[0] = '\0';
    }

    /// @brief Construct from string literal
    /// @tparam M Literal size (including null terminator)
    template<usize M> constexpr ArrayString(const char (&str)[M]) noexcept {
        static_assert(M > 0, "String literal must not be empty");
        assign(StringView(str, min(M - 1, N)));
    }

    /// @brief Construct from StringView
    constexpr explicit ArrayString(StringView view) noexcept {
        assign(view);
    }

    /// @brief Construct from C-string
    constexpr ArrayString(const char *str) noexcept {
        assign(StringView(str));
    }

    /// @brief Construct from single character repeated
    constexpr ArrayString(char ch, usize count) noexcept {
        assign(ch, count);
    }

    /// @brief Get C-style string pointer
    kf_nodiscard constexpr const char *c_str() const noexcept {
        return buffer_.data();
    }

    /// @brief Get string data pointer
    kf_nodiscard constexpr char *data() noexcept {
        return buffer_.data();
    }

    /// @brief Get const string data pointer
    kf_nodiscard constexpr const char *data() const noexcept {
        return buffer_.data();
    }

    /// @brief Get string as StringView
    kf_nodiscard constexpr StringView view() const noexcept {
        return StringView(buffer_.data(), size_);
    }

    /// @brief Get string as Slice
    kf_nodiscard constexpr Slice<const char> slice() const noexcept {
        return Slice<const char>{buffer_.data(), size_};
    }

    /// @brief Get mutable Slice (use with caution)
    kf_nodiscard constexpr Slice<char> slice_mut() noexcept {
        return Slice<char>{buffer_.data(), size_};
    }

    /// @brief Get current string length
    kf_nodiscard constexpr usize size() const noexcept {
        return size_;
    }

    /// @brief Get maximum capacity (excluding null terminator)
    kf_nodiscard constexpr usize capacity() const noexcept {
        return N;
    }

    /// @brief Check if string is empty
    kf_nodiscard constexpr bool empty() const noexcept {
        return size_ == 0;
    }

    /// @brief Check if string is full (no more characters can be added)
    kf_nodiscard constexpr bool full() const noexcept {
        return size_ == N;
    }

    /// @brief Clear string contents
    constexpr void clear() noexcept {
        size_ = 0;
        buffer_[0] = '\0';
    }

    /// @brief Assign string from StringView
    constexpr ArrayString &assign(StringView view) noexcept {
        size_ = min(view.size(), N);
        for (usize i = 0; i < size_; ++i) {
            buffer_[i] = view[i];
        }
        buffer_[size_] = '\0';
        return *this;
    }

    /// @brief Assign string from single character repeated
    constexpr ArrayString &assign(char ch, usize count) noexcept {
        size_ = min(count, N);
        for (usize i = 0; i < size_; ++i) {
            buffer_[i] = ch;
        }
        buffer_[size_] = '\0';
        return *this;
    }

    /// @brief Append character
    /// @param ch Character to append
    /// @return true if character was appended, false if buffer full
    kf_nodiscard constexpr bool push_back(char ch) noexcept {
        if (size_ >= N) { return false; }
        buffer_[size_] = ch;
        ++size_;
        buffer_[size_] = '\0';
        return true;
    }

    /// @brief Remove last character
    /// @return true if character was removed, false if string empty
    kf_nodiscard constexpr bool pop_back() noexcept {
        if (size_ == 0) { return false; }
        --size_;
        buffer_[size_] = '\0';
        return true;
    }

    /// @brief Append StringView
    /// @param view String to append
    /// @return Number of characters actually appended
    kf_nodiscard constexpr usize append(StringView view) noexcept {
        const usize available = N - size_;
        const usize to_append = min(view.size(), available);

        for (usize i = 0; i < to_append; ++i) {
            buffer_[size_ + i] = view[i];
        }

        size_ += to_append;
        buffer_[size_] = '\0';
        return to_append;
    }

    /// @brief Append C-string
    /// @param str C-string to append
    /// @return Number of characters actually appended
    kf_nodiscard constexpr usize append(const char *str) noexcept {
        return append(StringView(str));
    }

    /// @brief Insert string at position
    /// @param pos Position to insert at (0 <= pos <= size())
    /// @param view String to insert
    /// @return Number of characters inserted
    kf_nodiscard constexpr usize insert(usize pos, StringView view) noexcept {
        if (pos > size_) { pos = size_; }

        const usize available = N - size_;
        const usize to_insert = min(view.size(), available);

        // Make space for inserted characters
        for (usize i = size_; i > pos; --i) {
            buffer_[i + to_insert - 1] = buffer_[i - 1];
        }

        // Insert new characters
        for (usize i = 0; i < to_insert; ++i) {
            buffer_[pos + i] = view[i];
        }

        size_ += to_insert;
        buffer_[size_] = '\0';
        return to_insert;
    }

    /// @brief Erase characters from string
    /// @param pos Starting position
    /// @param count Number of characters to erase
    /// @return Number of characters erased
    kf_nodiscard constexpr usize erase(usize pos, usize count = 1) noexcept {
        if (pos >= size_) { return 0; }

        const usize remaining = size_ - pos;
        const usize to_erase = min(count, remaining);

        // Shift characters left
        for (usize i = pos; i < size_ - to_erase; ++i) {
            buffer_[i] = buffer_[i + to_erase];
        }

        size_ -= to_erase;
        buffer_[size_] = '\0';
        return to_erase;
    }

    /// @brief Format string using printf-style syntax
    /// @param format Format string
    /// @param ... Variable arguments matching format specifiers
    /// @return Number of characters written (excluding null terminator)
    /// @note Always null-terminates the result
    kf_nodiscard usize format(const char *format, ...) noexcept {
        if (N == 0) {
            buffer_[0] = '\0';
            return 0;
        }

        va_list args;
        va_start(args, format);

        // Use vsnprintf for safe formatting
        const int result = vsnprintf(buffer_.data(), N + 1, format, args);

        va_end(args);

        if (result < 0) {
            // Formatting error
            size_ = 0;
            buffer_[0] = '\0';
            return 0;
        }

        size_ = min(static_cast<usize>(result), N);
        buffer_[size_] = '\0';  // vsnprintf already null-terminates, but be safe
        return size_;
    }

    /// @brief Trim whitespace from beginning
    /// @return Reference to this string
    constexpr ArrayString &trim_start() noexcept {
        usize i = 0;
        while (i < size_ && isWhitespace(buffer_[i])) {
            ++i;
        }

        if (i > 0) {
            // Shift characters left
            for (usize j = 0; j < size_ - i; ++j) {
                buffer_[j] = buffer_[j + i];
            }
            size_ -= i;
            buffer_[size_] = '\0';
        }

        return *this;
    }

    /// @brief Trim whitespace from end
    /// @return Reference to this string
    constexpr ArrayString &trim_end() noexcept {
        while (size_ > 0 && isWhitespace(buffer_[size_ - 1])) {
            --size_;
        }
        buffer_[size_] = '\0';
        return *this;
    }

    /// @brief Trim whitespace from both ends
    /// @return Reference to this string
    constexpr ArrayString &trim() noexcept {
        return trim_start().trim_end();
    }

    /// @brief Find character in string
    /// @param ch Character to find
    /// @param pos Starting position
    /// @return Option containing position of character if found
    kf_nodiscard constexpr Option<usize> find(char ch, usize pos = 0) const noexcept {
        for (usize i = pos; i < size_; ++i) {
            if (buffer_[i] == ch) {
                return i;
            }
        }
        return {};
    }

    /// @brief Find substring in string
    /// @param str Substring to find
    /// @param pos Starting position
    /// @return Option containing position of substring if found
    kf_nodiscard constexpr Option<usize> find(StringView str, usize pos = 0) const noexcept {
        if (str.size() > size_ || pos > size_ - str.size()) {
            return {};
        }

        for (usize i = pos; i <= size_ - str.size(); ++i) {
            bool found = true;
            for (usize j = 0; j < str.size(); ++j) {
                if (buffer_[i + j] != str[j]) {
                    found = false;
                    break;
                }
            }
            if (found) {
                return i;
            }
        }
        return {};
    }

    /// @brief Check if string starts with prefix
    /// @param prefix Prefix to check
    /// @return true if string starts with prefix
    kf_nodiscard constexpr bool starts_with(StringView prefix) const noexcept {
        return view().startsWith(prefix);
    }

    /// @brief Check if string ends with suffix
    /// @param suffix Suffix to check
    /// @return true if string ends with suffix
    kf_nodiscard constexpr bool ends_with(StringView suffix) const noexcept {
        return view().endsWith(suffix);
    }

    /// @brief Get character at index (no bounds checking)
    kf_nodiscard constexpr char operator[](usize index) const noexcept {
        return buffer_[index];
    }

    /// @brief Get mutable character at index (no bounds checking)
    kf_nodiscard constexpr char &operator[](usize index) noexcept {
        return buffer_[index];
    }

    /// @brief Assignment from StringView
    constexpr ArrayString &operator=(StringView view) noexcept {
        return assign(view);
    }

    /// @brief Assignment from C-string
    constexpr ArrayString &operator=(const char *str) noexcept {
        return assign(StringView(str));
    }

    /// @brief Assignment from string literal
    template<usize M>
    constexpr ArrayString &operator=(const char (&str)[M]) noexcept {
        return assign(StringView(str, min(M - 1, N)));
    }

    /// @brief Append operator for character
    constexpr ArrayString &operator+=(char ch) noexcept {
        push_back(ch);
        return *this;
    }

    /// @brief Append operator for StringView
    constexpr ArrayString &operator+=(StringView view) noexcept {
        append(view);
        return *this;
    }

    /// @brief Append operator for C-string
    constexpr ArrayString &operator+=(const char *str) noexcept {
        append(str);
        return *this;
    }

    /// @brief Compare with StringView
    kf_nodiscard constexpr int compare(StringView other) const noexcept {
        return view().compare(other);
    }

    /// @brief Compare with C-string
    kf_nodiscard constexpr int compare(const char *str) const noexcept {
        return view().compare(StringView(str));
    }

    /// @brief Implicit conversion to StringView
    kf_nodiscard constexpr operator StringView() const noexcept {
        return view();
    }

    /// @brief Implicit conversion to const char*
    kf_nodiscard constexpr operator const char *() const noexcept {
        return c_str();
    }

private:
    /// @brief Check if character is whitespace
    static constexpr bool isWhitespace(char ch) noexcept {
        return ch == ' ' or ch == '\t' or ch == '\n' or ch == '\r' or ch == '\v' or ch == '\f';
    }
};

/// @brief Compare FixedString with StringView for equality
template<usize N> constexpr bool operator==(const ArrayString<N> &lhs, StringView rhs) noexcept {
    return lhs.view() == rhs;
}

/// @brief Compare StringView with FixedString for equality
template<usize N> constexpr bool operator==(StringView lhs, const ArrayString<N> &rhs) noexcept {
    return lhs == rhs.view();
}

/// @brief Compare FixedString with C-string for equality
template<usize N> constexpr bool operator==(const ArrayString<N> &lhs, const char *rhs) noexcept {
    return lhs.view() == StringView(rhs);
}

/// @brief Compare C-string with FixedString for equality
template<usize N> constexpr bool operator==(const char *lhs, const ArrayString<N> &rhs) noexcept {
    return StringView(lhs) == rhs.view();
}

/// @brief Inequality operators (implement via equality)
template<usize N, typename T> constexpr bool operator!=(const ArrayString<N> &lhs, const T &rhs) noexcept {
    return !(lhs == rhs);
}

template<usize N, typename T> constexpr bool operator!=(const T &lhs, const ArrayString<N> &rhs) noexcept {
    return !(lhs == rhs);
}

} // namespace kf
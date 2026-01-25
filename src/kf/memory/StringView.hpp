// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/memory/Slice.hpp"
#include "kf/core/attributes.hpp"
#include "kf/core/type_traits.hpp"
#include "kf/algorithm.hpp"
#include "kf/Option.hpp"


namespace kf {

/// @brief Lightweight non-owning string view (similar to std::string_view)
/// @note UTF-8 compatible, supports both null-terminated and sized strings
struct StringView {
private:
    const char *data_;
    usize size_;

public:
    /// @brief Default constructor (empty string)
    constexpr StringView() noexcept:
        data_{nullptr}, size_{0} {}

    /// @brief Construct from null-terminated C-string
    /// @param str Null-terminated string (must remain valid)
    constexpr StringView(const char *str) noexcept:
        data_{str}, size_{calculateSize(str)} {}

    /// @brief Construct from pointer and size
    /// @param data Pointer to string data
    /// @param size String length (not including null terminator if present)
    constexpr StringView(const char *data, usize size) noexcept:
        data_{data}, size_{size} {}

    /// @brief Construct from string literal (compile-time known size)
    /// @tparam N Compile-time string length (includes null terminator)
    /// @param str String literal
    template<usize N> constexpr StringView(const char (&str)[N]) noexcept :
        data_{str}, size_{N - 1} {}  // Exclude null terminator

    /// @brief Construct from Slice<const char>
    /// @param slice Slice containing string data
    constexpr explicit StringView(Slice<const char> slice) noexcept:
        data_{slice.data()}, size_{slice.size()} {}

    /// @brief Get pointer to string data
    kf_nodiscard constexpr const char *data() const noexcept { return data_; }

    /// @brief Get string size (excluding null terminator)
    kf_nodiscard constexpr usize size() const noexcept { return size_; }

    /// @brief Check if string is empty
    kf_nodiscard constexpr bool empty() const noexcept { return size_ == 0; }

    /// @brief Get character at index (no bounds checking)
    kf_nodiscard constexpr char operator[](usize index) const noexcept {
        return data_[index];
    }

    /// @brief Get first character
    kf_nodiscard constexpr char front() const noexcept {
        return data_[0];
    }

    /// @brief Get last character
    kf_nodiscard constexpr char back() const noexcept {
        return data_[size_ - 1];
    }

    /// @brief Get iterator to beginning
    kf_nodiscard constexpr const char *begin() const noexcept { return data_; }

    /// @brief Get iterator to end
    kf_nodiscard constexpr const char *end() const noexcept { return data_ + size_; }

    /// @brief Get string as Slice
    kf_nodiscard constexpr Slice<const char> slice() const noexcept {
        return Slice<const char>{data_, size_};
    }

    /// @brief Create sub-string view
    /// @param pos Starting position
    /// @param count Number of characters
    /// @return Sub-string view, empty if out of bounds
    kf_nodiscard constexpr StringView sub(usize pos, usize count) const noexcept {
        if (pos >= size_) { return StringView{}; }
        const usize actual_count = min(count, size_ - pos);
        return {data_ + pos, actual_count};
    }

    /// @brief Create sub-string view from position to end
    /// @param pos Starting position
    /// @return Sub-string view from pos to end, empty if out of bounds
    kf_nodiscard constexpr StringView subFrom(usize pos) const noexcept {
        if (pos >= size_) { return StringView{}; }
        return {data_ + pos, size_ - pos};
    }

    /// @brief Check if string starts with prefix
    /// @param prefix Prefix to check
    /// @return true if string starts with prefix
    kf_nodiscard constexpr bool startsWith(StringView prefix) const noexcept {
        if (prefix.size() > size_) { return false; }
        for (usize i = 0; i < prefix.size(); ++i) {
            if (data_[i] != prefix[i]) { return false; }
        }
        return true;
    }

    /// @brief Check if string ends with suffix
    /// @param suffix Suffix to check
    /// @return true if string ends with suffix
    kf_nodiscard constexpr bool endsWith(StringView suffix) const noexcept {
        if (suffix.size() > size_) { return false; }
        const usize offset = size_ - suffix.size();
        for (usize i = 0; i < suffix.size(); ++i) {
            if (data_[offset + i] != suffix[i]) { return false; }
        }
        return true;
    }

    /// @brief Compare with another string view
    /// @param other String to compare with
    /// @return Negative if less, zero if equal, positive if greater
    kf_nodiscard constexpr int compare(StringView other) const noexcept {
        const usize min_size = min(size_, other.size_);
        for (usize i = 0; i < min_size; ++i) {
            if (data_[i] != other.data_[i]) {
                return static_cast<int>(data_[i]) - static_cast<int>(other.data_[i]);
            }
        }
        return static_cast<int>(size_) - static_cast<int>(other.size_);
    }

    /// @brief Find character in string
    /// @param ch Character to find
    /// @param pos Starting position
    /// @return Option containing position of character if found, empty otherwise
    kf_nodiscard constexpr Option<usize> find(char ch, usize pos = 0) const noexcept {
        for (usize i = pos; i < size_; ++i) {
            if (data_[i] == ch) { return i; }
        }
        return {};
    }

    /// @brief Find substring
    /// @param str Substring to find
    /// @param pos Starting position
    /// @return Option containing position of substring if found, empty otherwise
    kf_nodiscard constexpr Option<usize> find(StringView str, usize pos = 0) const noexcept {
        if (str.size() > size_ or pos > size_ - str.size()) { return {}; }
        for (usize i = pos; i <= size_ - str.size(); ++i) {
            bool found = true;
            for (usize j = 0; j < str.size(); ++j) {
                if (data_[i + j] != str[j]) {
                    found = false;
                    break;
                }
            }
            if (found) { return i; }
        }
        return {};
    }

    /// @brief Find last occurrence of character
    /// @param ch Character to find
    /// @param pos Starting position (search backwards from this position)
    /// @return Option containing position of character if found, empty otherwise
    kf_nodiscard constexpr Option<usize> rfind(char ch, usize pos = static_cast<usize>(-1)) const noexcept {
        if (size_ == 0) { return {}; }

        usize start = (pos >= size_) ? size_ - 1 : pos;
        for (usize i = start; i != static_cast<usize>(-1); --i) {
            if (data_[i] == ch) { return i; }
        }
        return {};
    }

    /// @brief Remove prefix
    /// @param count Number of characters to remove from front
    constexpr void removePrefix(usize count) noexcept {
        if (count > size_) { count = size_; }
        data_ += count;
        size_ -= count;
    }

    /// @brief Remove suffix
    /// @param count Number of characters to remove from back
    constexpr void removeSuffix(usize count) noexcept {
        if (count > size_) { count = size_; }
        size_ -= count;
    }

    /// @brief Trim whitespace from beginning
    /// @return StringView with leading whitespace removed
    kf_nodiscard constexpr StringView trimStart() const noexcept {
        usize i = 0;
        while (i < size_ and isWhitespace(data_[i])) { ++i; }
        return subFrom(i);
    }

    /// @brief Trim whitespace from end
    /// @return StringView with trailing whitespace removed
    kf_nodiscard constexpr StringView trimEnd() const noexcept {
        if (size_ == 0) { return *this; }
        usize i = size_;
        while (i > 0 and isWhitespace(data_[i - 1])) { --i; }
        return {data_, i};
    }

    /// @brief Trim whitespace from both ends
    /// @return StringView with leading and trailing whitespace removed
    kf_nodiscard constexpr StringView trim() const noexcept {
        return trimStart().trimStart();
    }

private:
    /// @brief Calculate string length (safe for null pointers)
    static constexpr usize calculateSize(const char *str) noexcept {
        if (!str) { return 0; }
        usize size = 0;
        while (str[size] != '\0') { ++size; }
        return size;
    }

    /// @brief Check if character is whitespace
    static constexpr bool isWhitespace(char ch) noexcept {
        return ch == ' ' or ch == '\t' or ch == '\n' or ch == '\r';
    }
};

/// @brief Compare string views for equality
constexpr bool operator==(StringView lhs, StringView rhs) noexcept {
    return lhs.compare(rhs) == 0;
}

/// @brief Compare string views for inequality
constexpr bool operator!=(StringView lhs, StringView rhs) noexcept {
    return lhs.compare(rhs) != 0;
}

/// @brief Compare string views for less-than
constexpr bool operator<(StringView lhs, StringView rhs) noexcept {
    return lhs.compare(rhs) < 0;
}

/// @brief Compare string views for less-than-or-equal
constexpr bool operator<=(StringView lhs, StringView rhs) noexcept {
    return lhs.compare(rhs) <= 0;
}

/// @brief Compare string views for greater-than
constexpr bool operator>(StringView lhs, StringView rhs) noexcept {
    return lhs.compare(rhs) > 0;
}

/// @brief Compare string views for greater-than-or-equal
constexpr bool operator>=(StringView lhs, StringView rhs) noexcept {
    return lhs.compare(rhs) >= 0;
}


} // namespace kf
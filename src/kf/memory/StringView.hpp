// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include "kf/Option.hpp"
#include "kf/algorithm.hpp"
#include "kf/memory/Slice.hpp"

namespace kf::memory {

/// @brief Lightweight non-owning string view (similar to std::string_view)
/// @note UTF-8 compatible, supports both null-terminated and sized strings
struct StringView {

    /// @brief Default constructor (empty string)
    constexpr StringView() noexcept :
        _data{nullptr}, _size{0} {}

    /// @brief Construct from null-terminated C-string
    /// @param str Null-terminated string (must remain valid)
    constexpr StringView(const char *str) noexcept :
        _data{str}, _size{calculateSize(str)} {}

    /// @brief Construct from pointer and size
    /// @param data Pointer to string data
    /// @param size String length (not including null terminator if present)
    constexpr StringView(const char *data, usize size) noexcept :
        _data{data}, _size{size} {}

    /// @brief Construct from string literal (compile-time known size)
    /// @tparam N Compile-time string length (includes null terminator)
    /// @param str String literal
    template<usize N> constexpr StringView(const char (&str)[N]) noexcept :
        _data{str}, _size{N - 1} {}// Exclude null terminator

    /// @brief Construct from Slice<const char>
    /// @param slice Slice containing string data
    constexpr explicit StringView(Slice<const char> slice) noexcept :
        _data{slice.data()}, _size{slice.size()} {}

    /// @brief Get pointer to string data
    [[nodiscard]] constexpr const char *data() const noexcept { return _data; }

    /// @brief Get string size (excluding null terminator)
    [[nodiscard]] constexpr usize size() const noexcept { return _size; }

    /// @brief Check if string is empty
    [[nodiscard]] constexpr bool empty() const noexcept { return _size == 0; }

    /// @brief Get character at index (no bounds checking)
    [[nodiscard]] constexpr char operator[](usize index) const noexcept {
        return _data[index];
    }

    /// @brief Get first character
    [[nodiscard]] constexpr char front() const noexcept {
        return _data[0];
    }

    /// @brief Get last character
    [[nodiscard]] constexpr char back() const noexcept {
        return _data[_size - 1];
    }

    /// @brief Get iterator to beginning
    [[nodiscard]] constexpr const char *begin() const noexcept { return _data; }

    /// @brief Get iterator to end
    [[nodiscard]] constexpr const char *end() const noexcept { return _data + _size; }

    /// @brief Get string as Slice
    [[nodiscard]] constexpr Slice<const char> slice() const noexcept {
        return Slice<const char>{_data, _size};
    }

    /// @brief Create sub-string view
    /// @param pos Starting position
    /// @param count Number of characters
    /// @return Sub-string view, empty if out of bounds
    [[nodiscard]] constexpr StringView sub(usize pos, usize count) const noexcept {
        if (pos >= _size) { return StringView{}; }
        const usize actual_count = min(count, _size - pos);
        return {_data + pos, actual_count};
    }

    /// @brief Create sub-string view from position to end
    /// @param pos Starting position
    /// @return Sub-string view from pos to end, empty if out of bounds
    [[nodiscard]] constexpr StringView subFrom(usize pos) const noexcept {
        if (pos >= _size) { return StringView{}; }
        return {_data + pos, _size - pos};
    }

    /// @brief Check if string starts with prefix
    /// @param prefix Prefix to check
    /// @return true if string starts with prefix
    [[nodiscard]] constexpr bool startsWith(StringView prefix) const noexcept {
        if (prefix.size() > _size) { return false; }
        for (usize i = 0; i < prefix.size(); ++i) {
            if (_data[i] != prefix[i]) { return false; }
        }
        return true;
    }

    /// @brief Check if string ends with suffix
    /// @param suffix Suffix to check
    /// @return true if string ends with suffix
    [[nodiscard]] constexpr bool endsWith(StringView suffix) const noexcept {
        if (suffix.size() > _size) { return false; }
        const usize offset = _size - suffix.size();
        for (usize i = 0; i < suffix.size(); ++i) {
            if (_data[offset + i] != suffix[i]) { return false; }
        }
        return true;
    }

    /// @brief Compare with another string view
    /// @param other String to compare with
    /// @return Negative if less, zero if equal, positive if greater
    [[nodiscard]] constexpr int compare(StringView other) const noexcept {
        const usize min_size = min(_size, other._size);
        for (usize i = 0; i < min_size; ++i) {
            if (_data[i] != other._data[i]) {
                return static_cast<int>(_data[i]) - static_cast<int>(other._data[i]);
            }
        }
        return static_cast<int>(_size) - static_cast<int>(other._size);
    }

    /// @brief Find character in string
    /// @param ch Character to find
    /// @param pos Starting position
    /// @return Option containing position of character if found, empty otherwise
    [[nodiscard]] constexpr Option<usize> find(char ch, usize pos = 0) const noexcept {
        for (usize i = pos; i < _size; ++i) {
            if (_data[i] == ch) { return some(i); }
        }
        return none;
    }

    /// @brief Find substring
    /// @param str Substring to find
    /// @param pos Starting position
    /// @return Option containing position of substring if found, empty otherwise
    [[nodiscard]] constexpr Option<usize> find(StringView str, usize pos = 0) const noexcept {
        if (str.size() > _size or pos > _size - str.size()) { return none; }
        for (usize i = pos; i <= _size - str.size(); ++i) {
            bool found = true;
            for (usize j = 0; j < str.size(); ++j) {
                if (_data[i + j] != str[j]) {
                    found = false;
                    break;
                }
            }
            if (found) { return some(i); }
        }
        return none;
    }

    /// @brief Find last occurrence of character
    /// @param ch Character to find
    /// @param pos Starting position (search backwards from this position)
    /// @return Option containing position of character if found, empty otherwise
    [[nodiscard]] constexpr Option<usize> rfind(char ch, usize pos = static_cast<usize>(-1)) const noexcept {
        if (_size == 0) { return none; }

        usize start = (pos >= _size) ? _size - 1 : pos;
        for (usize i = start; i != static_cast<usize>(-1); --i) {
            if (_data[i] == ch) { return some(i); }
        }
        return none;
    }

    /// @brief Remove prefix
    /// @param count Number of characters to remove from front
    constexpr void removePrefix(usize count) noexcept {
        if (count > _size) { count = _size; }
        _data += count;
        _size -= count;
    }

    /// @brief Remove suffix
    /// @param count Number of characters to remove from back
    constexpr void removeSuffix(usize count) noexcept {
        if (count > _size) { count = _size; }
        _size -= count;
    }

    /// @brief Trim whitespace from beginning
    /// @return StringView with leading whitespace removed
    [[nodiscard]] constexpr StringView trimStart() const noexcept {
        usize i = 0;
        while (i < _size and isWhitespace(_data[i])) { ++i; }
        return subFrom(i);
    }

    /// @brief Trim whitespace from end
    /// @return StringView with trailing whitespace removed
    [[nodiscard]] constexpr StringView trimEnd() const noexcept {
        if (_size == 0) { return *this; }
        usize i = _size;
        while (i > 0 and isWhitespace(_data[i - 1])) { --i; }
        return {_data, i};
    }

    /// @brief Trim whitespace from both ends
    /// @return StringView with leading and trailing whitespace removed
    [[nodiscard]] constexpr StringView trim() const noexcept {
        return trimStart().trimEnd();
    }

private:
    const char *_data;
    usize _size;

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

}// namespace kf::memory
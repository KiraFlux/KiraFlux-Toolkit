// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    StringView.hpp

#pragma once

#include "kf/Option.hpp"
#include "kf/Slice.hpp"
#include "kf/core.hpp"

#include "kf/mixin/Equatable.hpp"

namespace kf {

namespace internal {

using StringViewBase = Slice<char const>;

}

/// @brief   Non‑owning view of a constant string with trimming, searching, and splitting.
/// @note Inherits `Slice<const char>` and adds string‑specific operations
struct StringView : internal::StringViewBase, mixin::Equatable<StringView> {

    using internal::StringViewBase::StringViewBase;

    /// @brief Construct from char slice
    constexpr StringView(internal::StringViewBase buffer) noexcept :
        internal::StringViewBase{buffer} {}

    /// @brief Construct from C‑string (nullptr allowed)
    constexpr StringView(char const *str) noexcept :
        internal::StringViewBase{str, (nullptr == str) ? 0 : cStringLength(str)} {}

    // Trimming

    /// @brief Trim leading whitespace
    [[nodiscard]] constexpr StringView trimStart() const noexcept {
        auto i = 0u;

        while (i < this->length() and isWhitespace((*this)[i])) {
            i += 1;
        }

        return {data() + i, this->length() - i};
    }

    /// @brief Trim trailing whitespace
    [[nodiscard]] constexpr StringView trimEnd() const noexcept {
        auto i = this->length();

        while (i > 0 and isWhitespace((*this)[i - 1])) {
            i -= 1;
        }

        return {data(), i};
    }

    /// @brief Trim both ends
    [[nodiscard]] constexpr StringView trim() const noexcept {
        return trimStart().trimEnd();
    }

    // Checks

    [[nodiscard]] constexpr bool startsWith(StringView prefix) const noexcept {
        if (prefix.length() > this->length()) {
            return false;
        }

        for (auto i = 0u; i < prefix.length(); i += 1) {
            if ((*this)[i] != prefix[i]) {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] constexpr bool endsWith(StringView suffix) const noexcept {
        if (suffix.length() > this->length()) {
            return false;
        }

        auto offset = this->length() - suffix.length();

        for (usize i = 0; i < suffix.length(); i += 1) {
            if ((*this)[offset + i] != suffix[i]) {
                return false;
            }
        }

        return true;
    }

    [[nodiscard]] constexpr bool contains(StringView sub) const noexcept {
        return indexOf(sub).isSome();
    }

    [[nodiscard]] constexpr bool contains(char c) const noexcept {
        return indexOf(c).isSome();
    }

    // Search

    [[nodiscard]] constexpr auto indexOf(char c) const noexcept -> Option<usize> {
        for (usize i = 0; i < this->length(); i += 1) {
            if ((*this)[i] == c) {
                return some(i);
            }
        }

        return none;
    }

    [[nodiscard]] constexpr auto lastIndexOf(char c) const noexcept -> Option<usize> {
        for (usize i = this->length(); i > 0; i -= 1) {
            if ((*this)[i - 1] == c) {
                return some(i - 1);
            }
        }

        return none;
    }

    [[nodiscard]] constexpr auto indexOf(StringView sub) const noexcept -> Option<usize> {
        if (sub.empty() or sub.length() > this->length()) {
            return none;
        }

        for (usize i = 0; i <= this->length() - sub.length(); i += 1) {
            bool ok = true;

            for (auto j = 0u; j < sub.length(); j += 1)
                if ((*this)[i + j] != sub[j]) {
                    ok = false;
                    break;
                }

            if (ok) {
                return some(i);
            }
        }

        return none;
    }

    [[nodiscard]] constexpr auto lastIndexOf(StringView sub) const noexcept -> Option<usize> {
        if (sub.empty() or sub.length() > this->length()) {
            return none;
        }

        for (usize i = this->length() - sub.length(); i > 0; i -= 1) {
            bool ok = true;

            for (usize j = 0u; j < sub.length(); j += 1)
                if ((*this)[i + j] != sub[j]) {
                    ok = false;
                    break;
                }

            if (ok) {
                return some(i);
            }
        }

        return none;
    }

    // Remove prefix/suffix

    [[nodiscard]] constexpr StringView removePrefix(StringView prefix) const noexcept {
        return startsWith(prefix) ? StringView{data() + prefix.length(), this->length() - prefix.length()} : *this;
    }

    [[nodiscard]] constexpr StringView removeSuffix(StringView suffix) const noexcept {
        return endsWith(suffix) ? StringView{data(), this->length() - suffix.length()} : *this;
    }

    // Split

    /// @brief Split by any character from `delimiters`
    /// @param buffer      Output buffer for tokens
    /// @param delimiters  Set of delimiter characters (default whitespace)
    /// @param keep_empty  If true, empty tokens are included; otherwise skipped
    /// @return Slice of filled tokens (may be smaller than buffer if fewer tokens)
    [[nodiscard]] constexpr auto split(Slice<StringView> buffer, StringView delimiters = " \t\n\r\v\f", bool keep_empty = false) const noexcept -> Slice<StringView> {
        if (buffer.empty() or delimiters.empty()) {
            return {};
        }

        auto token_count = 0u, start = 0u;

        for (usize i = 0; i <= this->length(); i += 1) {
            bool is_delim = (i < this->length() and delimiters.contains((*this)[i]));

            if (i == this->length() or is_delim) {
                if (i > start or keep_empty) {
                    if (token_count < buffer.length()) {
                        buffer[token_count] = {data() + start, i - start};
                        token_count += 1;
                    } else {
                        // Buffer full: last token holds the rest (including delimiters)
                        buffer[token_count - 1] = {data() + start, this->length() - start};
                        break;
                    }
                }
                start = i + 1;
            }
        }

        return {buffer.data(), token_count};
    }

private:
    [[nodiscard]] static constexpr bool isWhitespace(char c) noexcept {
        return c == ' ' or c == '\t' or c == '\n' or c == '\r' or c == '\v' or c == '\f';
    }

    [[nodiscard]] static constexpr usize cStringLength(char const *str) noexcept {
        usize len = 0;

        while (str[len] != '\0') {
            len += 1;
        }

        return len;
    }

    KF_IMPL_EQUATABLE(StringView);
    constexpr bool isEqualsImpl(StringView const &other) const noexcept {
        if (this->length() == other.length()) {

            for (usize i = 0; i < this->length(); i += 1) {
                if ((*this)[i] != other[i]) {
                    return false;
                }
            }
            return true;

        } else {
            return false;
        }
    }
};

}// namespace kf
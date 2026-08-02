// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    Slice.hpp

#pragma once

#include "kf/Option.hpp"
#include "kf/Sequence.hpp"
#include "kf/primitives.hpp"

namespace kf {

/// @brief   Non‑owning view of a contiguous memory region with slicing and iteration.
/// @tparam T Element type
template<typename T> struct Slice : Sequence<Slice<T>, T> {

    /// @brief Construct empty slice
    constexpr Slice() noexcept :
        _ptr{nullptr}, _length{0} {}

    /// @brief Construct slice from pointer and length
    /// @param ptr Pointer to first element
    /// @param length Number of elements
    constexpr Slice(T *ptr, usize length) noexcept :
        _ptr{ptr}, _length{length} {}

    /// @brief Array constructor
    /// @tparam N auto-deducted array length
    template<usize N> constexpr Slice(T (&arr)[N]) noexcept :
        _ptr{arr}, _length{N} {}

    /// @brief Create sub-slice starting at offset
    /// @param offset Starting position
    /// @param count Number of elements
    /// @return Slice covering specified range
    [[nodiscard]] constexpr Slice sub(usize offset, Option<usize> count = none) const noexcept {
        if (offset > _length) {
            return {};
        }

        usize len = count.isNone() ? (_length - offset) : count.unwrap();

        if (len > _length - offset) {
            len = _length - offset;
        }

        if (0 == len) {
            return {};
        }

        return {_ptr + offset, len};
    }

    /// @brief Get first N elements of slice
    /// @param n Number of elements from start
    /// @return Slice containing first n elements
    /// @note No bounds checking - caller must ensure n <= length()
    [[nodiscard]] constexpr Slice first(usize n) const noexcept {
        return sub(0, some(n));
    }

    /// @brief Get last N elements of slice
    /// @param n Number of elements from end
    /// @return Slice containing last n elements
    /// @note No bounds checking - caller must ensure n <= length()
    [[nodiscard]] constexpr Slice last(usize n) const noexcept {
        return sub(_length - n, some(n));
    }

    /// @brief Get slice starting from offset to end
    /// @param offset Starting position (must be <= length())
    /// @return Slice from offset to end of original slice
    /// @note No bounds checking - caller must ensure offset <= length()
    [[nodiscard]] constexpr Slice fromOffset(usize offset) const noexcept {
        return sub(offset, some(_length - offset));
    }

    constexpr operator Slice<T const>() const noexcept {
        return Slice<T const>{_ptr, _length};
    }

private:
    T *_ptr;      ///< Pointer to the first element
    usize _length;///< Number of elements in the slice

    KF_IMPL_SEQUENCE(Slice<T>, T);

    constexpr T *getDataImpl() noexcept {
        return _ptr;
    }

    constexpr usize lengthImpl() const noexcept {
        return _length;
    }
};

}// namespace kf
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Sequence.hpp"
#include "kf/primitives.hpp"

namespace kf {

/// @brief Non-owning view of a contiguous memory region
/// @tparam T Element type
template<typename T> struct Slice : Sequence<Slice<T>, T> {

    /// @brief Construct empty slice
    constexpr Slice() noexcept :
        _ptr{nullptr}, _size{0} {}

    /// @brief Construct slice from pointer and size
    /// @param ptr Pointer to first element
    /// @param size Number of elements
    constexpr Slice(T *ptr, usize size) noexcept :
        _ptr{ptr}, _size{size} {}

    /// @brief Array constructor
    /// @tparam N auto-deducted array length
    template<usize N> constexpr Slice(T (&arr)[N]) noexcept :
        _ptr{arr}, _size{N} {}

    /// @brief Create sub-slice starting at offset
    /// @param offset Starting position (must be <= size())
    /// @param count Number of elements (offset + count must be <= size())
    /// @return Slice covering specified range
    /// @note No bounds checking - caller must ensure valid range
    [[nodiscard]] constexpr Slice sub(usize offset, usize count) const noexcept {
        return Slice{_ptr + offset, count};
    }

    /// @brief Get first N elements of slice
    /// @param n Number of elements from start
    /// @return Slice containing first n elements
    /// @note No bounds checking - caller must ensure n <= size()
    [[nodiscard]] constexpr Slice first(usize n) const noexcept {
        return sub(0, n);
    }

    /// @brief Get last N elements of slice
    /// @param n Number of elements from end
    /// @return Slice containing last n elements
    /// @note No bounds checking - caller must ensure n <= size()
    [[nodiscard]] constexpr Slice last(usize n) const noexcept {
        return sub(_size - n, n);
    }

    /// @brief Get slice starting from offset to end
    /// @param offset Starting position (must be <= size())
    /// @return Slice from offset to end of original slice
    /// @note No bounds checking - caller must ensure offset <= size()
    [[nodiscard]] constexpr Slice fromOffset(usize offset) const noexcept {
        return sub(offset, _size - offset);
    }

    constexpr operator Slice<const T>() const noexcept {
        return Slice<const T>{_ptr, _size};
    }

private:
    T *_ptr;    ///< Pointer to the first element
    usize _size;///< Number of elements in the slice

    KF_IMPL_SEQUENCE(Slice<T>, T);

    constexpr T *getDataImpl() noexcept {
        return _ptr;
    }

    constexpr usize lengthImpl() const noexcept {
        return _size;
    }
};

}// namespace kf
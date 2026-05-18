// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/primitives.hpp"

namespace kf::memory {

/// @brief Non-owning view of a contiguous memory region
/// @tparam T Element type
/// @note Similar to std::span but for embedded use without exceptions
template<typename T> struct Slice {

private:
    T *_ptr;    ///< Pointer to the first element
    usize _size;///< Number of elements in the slice

public:
    /// @brief Default constructor (empty slice)
    constexpr Slice() noexcept :
        _ptr{nullptr}, _size{0} {}

    /// @brief Construct slice from pointer and size
    /// @param ptr Pointer to first element
    /// @param size Number of elements
    constexpr Slice(T *ptr, usize size) noexcept :
        _ptr{ptr}, _size{size} {}

    /// @brief Array constructor
    /// @tparam N auto-deducted array length
    template<usize N> constexpr Slice(T (&arr)[N]) noexcept : _ptr{arr}, _size{N} {}

    /// @brief Get iterator to beginning
    /// @return Iterator to first element
    [[nodiscard]] constexpr T *begin() noexcept { return _ptr; }

    /// @brief Get iterator to end
    /// @return Iterator to position after last element
    [[nodiscard]] constexpr T *end() noexcept { return _ptr + _size; }

    /// @brief Get const iterator to beginning
    /// @return Const iterator to first element
    [[nodiscard]] constexpr const T *begin() const noexcept { return _ptr; }

    /// @brief Get const iterator to end
    /// @return Const iterator to position after last element
    [[nodiscard]] constexpr const T *end() const noexcept { return _ptr + _size; }

    /// @brief Get pointer to underlying data
    /// @return Pointer to first element
    [[nodiscard]] constexpr T *data() noexcept { return _ptr; }

    /// @brief Get const pointer to underlying data
    /// @return Const pointer to first element
    [[nodiscard]] constexpr const T *data() const noexcept { return _ptr; }

    /// @brief Get number of elements in slice
    /// @return Size of slice in elements
    [[nodiscard]] constexpr usize size() const noexcept { return _size; }

    /// @brief Check if slice is empty
    /// @return true if slice contains no elements
    [[nodiscard]] constexpr bool empty() const noexcept { return _size == 0; }

    /// @brief Access element at index without bounds checking
    /// @param index Element position (0-based)
    /// @return Reference to element at index
    /// @warning No bounds checking performed
    [[nodiscard]] T &operator[](usize index) noexcept {
        return _ptr[index];
    }

    /// @brief Access element at index without bounds checking (const version)
    /// @param index Element position (0-based)
    /// @return Const reference to element at index
    /// @warning No bounds checking performed
    [[nodiscard]] const T &operator[](usize index) const noexcept {
        return _ptr[index];
    }

    /// @brief Create sub-slice starting at offset
    /// @param offset Starting position (must be <= size())
    /// @param count Number of elements (offset + count must be <= size())
    /// @return Slice covering specified range
    /// @note No bounds checking - caller must ensure valid range
    [[nodiscard]] Slice sub(usize offset, usize count) const noexcept {
        return Slice(_ptr + offset, count);
    }

    /// @brief Get first N elements of slice
    /// @param n Number of elements from start
    /// @return Slice containing first n elements
    /// @note No bounds checking - caller must ensure n <= size()
    [[nodiscard]] Slice first(usize n) const noexcept {
        return sub(0, n);
    }

    /// @brief Get last N elements of slice
    /// @param n Number of elements from end
    /// @return Slice containing last n elements
    /// @note No bounds checking - caller must ensure n <= size()
    [[nodiscard]] Slice last(usize n) const noexcept {
        return sub(_size - n, n);
    }

    /// @brief Get slice starting from offset to end
    /// @param offset Starting position (must be <= size())
    /// @return Slice from offset to end of original slice
    /// @note No bounds checking - caller must ensure offset <= size()
    [[nodiscard]] Slice fromOffset(usize offset) const noexcept {
        return sub(offset, _size - offset);
    }

    constexpr operator Slice<const T>() const noexcept {
        return Slice<const T>{_ptr, _size};
    }
};

}// namespace kf
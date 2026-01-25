// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/aliases.hpp"
#include "kf/core/attributes.hpp"


namespace kf {

/// @brief Non-owning view of a contiguous memory region
/// @tparam T Element type
/// @note Similar to std::span but for embedded use without exceptions
template<typename T> struct Slice {

private:
    T *ptr_;    ///< Pointer to the first element
    usize size_;///< Number of elements in the slice

public:
    /// @brief Default constructor (empty slice)
    constexpr Slice() noexcept:
        ptr_{nullptr}, size_{0} {}

    /// @brief Construct slice from pointer and size
    /// @param ptr Pointer to first element
    /// @param size Number of elements
    constexpr Slice(T *ptr, usize size) noexcept:
        ptr_{ptr}, size_{size} {}

    /// @brief Get iterator to beginning
    /// @return Iterator to first element
    kf_nodiscard constexpr T *begin() noexcept { return ptr_; }

    /// @brief Get iterator to end
    /// @return Iterator to position after last element
    kf_nodiscard constexpr T *end() noexcept { return ptr_ + size_; }

    /// @brief Get const iterator to beginning
    /// @return Const iterator to first element
    kf_nodiscard constexpr const T *begin() const noexcept { return ptr_; }

    /// @brief Get const iterator to end
    /// @return Const iterator to position after last element
    kf_nodiscard constexpr const T *end() const noexcept { return ptr_ + size_; }

    /// @brief Get pointer to underlying data
    /// @return Pointer to first element
    kf_nodiscard constexpr T *data() noexcept { return ptr_; }

    /// @brief Get const pointer to underlying data
    /// @return Const pointer to first element
    kf_nodiscard constexpr const T *data() const noexcept { return ptr_; }

    /// @brief Get number of elements in slice
    /// @return Size of slice in elements
    kf_nodiscard constexpr usize size() const noexcept { return size_; }

    /// @brief Check if slice is empty
    /// @return true if slice contains no elements
    kf_nodiscard constexpr bool empty() const noexcept { return size_ == 0; }

    /// @brief Access element at index without bounds checking
    /// @param index Element position (0-based)
    /// @return Reference to element at index
    /// @warning No bounds checking performed
    kf_nodiscard T &operator[](usize index) noexcept {
        return ptr_[index];
    }

    /// @brief Access element at index without bounds checking (const version)
    /// @param index Element position (0-based)
    /// @return Const reference to element at index
    /// @warning No bounds checking performed
    kf_nodiscard const T &operator[](usize index) const noexcept {
        return ptr_[index];
    }

    /// @brief Create sub-slice starting at offset
    /// @param offset Starting position (must be <= size())
    /// @param count Number of elements (offset + count must be <= size())
    /// @return Slice covering specified range
    /// @note No bounds checking - caller must ensure valid range
    kf_nodiscard Slice sub(usize offset, usize count) const noexcept {
        return Slice(ptr_ + offset, count);
    }

    /// @brief Get first N elements of slice
    /// @param n Number of elements from start
    /// @return Slice containing first n elements
    /// @note No bounds checking - caller must ensure n <= size()
    kf_nodiscard Slice first(usize n) const noexcept {
        return sub(0, n);
    }

    /// @brief Get last N elements of slice
    /// @param n Number of elements from end
    /// @return Slice containing last n elements
    /// @note No bounds checking - caller must ensure n <= size()
    kf_nodiscard Slice last(usize n) const noexcept {
        return sub(size_ - n, n);
    }

    /// @brief Get slice starting from offset to end
    /// @param offset Starting position (must be <= size())
    /// @return Slice from offset to end of original slice
    /// @note No bounds checking - caller must ensure offset <= size()
    kf_nodiscard Slice fromOffset(usize offset) const noexcept {
        return sub(offset, size_ - offset);
    }
};

}// namespace kf
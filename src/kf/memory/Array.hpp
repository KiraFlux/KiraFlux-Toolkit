// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once
#include "kf/core/config.hpp"

#if kf_port_has_array
#include <array>

namespace kf {

template<typename T, size_t N> using Array = std::array<T, N>;

}

#else

#include "stddef.h"// NOLINT(*-deprecated-headers)

#include "kf/core/attributes.hpp"
#include "kf/core/type_traits.hpp"

namespace kf {

/// @brief Fixed-size array implementation for AVR/embedded platforms
/// @tparam T Element type
/// @tparam N Array size (must be > 0)
/// @note Used when standard library is unavailable (kf_port_has_array is false)
template<typename T, size_t N>
struct Array {
    static_assert(N > 0, "Array size (N) must be >= 0");

    using value_type = T;                        ///< Element type
    using size_type = size_t;                    ///< Size type
    using difference_type = ptrdiff_t;           ///< Pointer difference type
    using reference = value_type &;              ///< Element reference
    using const_reference = const value_type &;  ///< Constant element reference
    using pointer = value_type *;                ///< Pointer to element
    using const_pointer = const value_type *;    ///< Constant pointer to element
    using iterator = pointer;                    ///< Iterator type
    using const_iterator = const_pointer;        ///< Constant iterator type
    using reverse_iterator = pointer;            ///< Simplified reverse iterator
    using const_reverse_iterator = const_pointer;///< Simplified constant reverse iterator

private:
    value_type data_[N];///< Internal storage array

public:
    /// @brief Access element with bounds checking
    /// @param pos Element position
    /// @return Reference to element at position
    /// @note No exception throwing on AVR (embedded constraint)
    reference at(size_type pos) {
        return data_[pos];
    }

    /// @brief Access element with bounds checking (const version)
    /// @param pos Element position
    /// @return Const reference to element at position
    const_reference at(size_type pos) const {
        return data_[pos];
    }

    /// @brief Access element without bounds checking
    /// @param pos Element position
    /// @return Reference to element at position
    reference operator[](size_type pos) {
        return data_[pos];
    }

    /// @brief Access element without bounds checking (const version)
    /// @param pos Element position
    /// @return Const reference to element at position
    const_reference operator[](size_type pos) const {
        return data_[pos];
    }

    /// @brief Access first element
    /// @return Reference to first element
    reference front() {
        return data_[0];
    }

    /// @brief Access first element (const version)
    /// @return Const reference to first element
    const_reference front() const {
        return data_[0];
    }

    /// @brief Access last element
    /// @return Reference to last element
    reference back() {
        return data_[N - 1];
    }

    /// @brief Access last element (const version)
    /// @return Const reference to last element
    const_reference back() const {
        return data_[N - 1];
    }

    /// @brief Direct access to underlying array
    /// @return Pointer to first element
    pointer data() noexcept {
        return data_;
    }

    /// @brief Direct access to underlying array (const version)
    /// @return Const pointer to first element
    const_pointer data() const noexcept {
        return data_;
    }

    /// @brief Iterator to beginning
    /// @return Iterator to first element
    iterator begin() noexcept {
        return data_;
    }

    /// @brief Iterator to beginning (const version)
    /// @return Const iterator to first element
    const_iterator begin() const noexcept {
        return data_;
    }

    /// @brief Const iterator to beginning
    /// @return Const iterator to first element
    const_iterator cbegin() const noexcept {
        return data_;
    }

    /// @brief Iterator to end
    /// @return Iterator to position after last element
    iterator end() noexcept {
        return data_ + N;
    }

    /// @brief Iterator to end (const version)
    /// @return Const iterator to position after last element
    const_iterator end() const noexcept {
        return data_ + N;
    }

    /// @brief Const iterator to end
    /// @return Const iterator to position after last element
    const_iterator cend() const noexcept {
        return data_ + N;
    }

    /// @brief Reverse iterator to beginning (simplified for AVR)
    /// @return Reverse iterator to last element
    iterator rbegin() noexcept {
        return data_ + N - 1;
    }

    /// @brief Reverse iterator to beginning (const version)
    /// @return Const reverse iterator to last element
    const_iterator rbegin() const noexcept {
        return data_ + N - 1;
    }

    /// @brief Reverse iterator to end (simplified for AVR)
    /// @return Reverse iterator to position before first element
    iterator rend() noexcept {
        return data_ - 1;
    }

    /// @brief Reverse iterator to end (const version)
    /// @return Const reverse iterator to position before first element
    const_iterator rend() const noexcept {
        return data_ - 1;
    }

    /// @brief Check if array is empty
    /// @return Always false for Array with N > 0
    kf_nodiscard constexpr bool empty() const noexcept {
        return N == 0;
    }

    /// @brief Get array size
    /// @return Number of elements in array (N)
    kf_nodiscard constexpr size_type size() const noexcept {
        return N;
    }

    /// @brief Get maximum possible size
    /// @return Same as size() for fixed-size array
    kf_nodiscard constexpr size_type max_size() const noexcept {
        return N;
    }

    /// @brief Fill array with specified value
    /// @param value Value to assign to all elements
    void fill(const T &value) {
        for (size_type i = 0; i < N; ++i) {
            data_[i] = value;
        }
    }

    /// @brief Swap contents with another array of same type and size
    /// @param other Array to swap with
    void swap(Array &other) {
        for (size_type i = 0; i < N; ++i) {
            swap(data_[i], other.data_[i]);
        }
    }

    /// @brief Equality comparison
    /// @param other Array to compare with
    /// @return true if all elements are equal
    bool operator==(const Array &other) const {
        for (size_type i = 0; i < N; ++i) {
            if (data_[i] == other.data_[i]) continue;
            return false;
        }
        return true;
    }

    /// @brief Inequality comparison
    /// @param other Array to compare with
    /// @return true if any element differs
    bool operator!=(const Array &other) const {
        return !(*this == other);// NOLINT(*-simplify)
    }

    /// @brief Less-than comparison (lexicographical)
    /// @param other Array to compare with
    /// @return true if this array is lexicographically less than other
    bool operator<(const Array &other) const {
        for (size_type i = 0; i < N; ++i) {
            if (data_[i] < other.data_[i]) return true;
            if (other.data_[i] < data_[i]) return false;
        }
        return false;
    }

    /// @brief Less-than-or-equal comparison (lexicographical)
    /// @param other Array to compare with
    /// @return true if this array is lexicographically less than or equal to other
    bool operator<=(const Array &other) const {
        return !(other < *this);// NOLINT(*-simplify)
    }

    /// @brief Greater-than comparison (lexicographical)
    /// @param other Array to compare with
    /// @return true if this array is lexicographically greater than other
    bool operator>(const Array &other) const {
        return other < *this;
    }

    /// @brief Greater-than-or-equal comparison (lexicographical)
    /// @param other Array to compare with
    /// @return true if this array is lexicographically greater than or equal to other
    bool operator>=(const Array &other) const {
        return !(*this < other);// NOLINT(*-simplify)
    }
};

}// namespace kf

#endif
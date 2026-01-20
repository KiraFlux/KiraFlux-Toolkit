// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/config.hpp"
#include "kf/memory/Allocator.hpp"

#if kf_port_has_vector
#include <vector>

namespace kf {

template<typename T, typename Alloc = kf::Allocator<T>> using ArrayList = std::vector<T, Alloc>;

}

#else

#include "kf/algorithm.hpp"
#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"
#include "kf/core/type_traits.hpp"
#include "kf/core/utility.hpp"

namespace kf {

/// @brief Dynamic array implementation for embedded systems without STL
/// @tparam T Element type
/// @tparam Alloc Allocator type (default: kf::allocator<T>)
/// @note Provides basic vector-like functionality with manual memory management
template<typename T, typename Alloc = kf::allocator<T>>
class ArrayList {
private:
    T *data_ = nullptr; ///< Pointer to dynamically allocated array
    usize size_ = 0;    ///< Current number of elements
    usize capacity_ = 0;///< Current allocated capacity
    Alloc alloc_;       ///< Allocator instance

public:
    using value_type = T;             ///< Element type
    using pointer = T *;              ///< Pointer to element
    using reference = T &;            ///< Reference to element
    using const_reference = const T &;///< Const reference to element
    using size_type = usize;          ///< Size type

    /// @brief Default constructor (empty array)
    ArrayList() = default;

    /// @brief Destructor - destroys all elements and deallocates memory
    ~ArrayList() {
        clear();
        if (data_) {
            alloc_.deallocate(data_, capacity_);
        }
    }

    /// @brief Access element without bounds checking
    /// @param pos Element index
    /// @return Reference to element at index
    reference operator[](size_type pos) { return data_[pos]; }

    /// @brief Access element without bounds checking (const version)
    /// @param pos Element index
    /// @return Const reference to element at index
    const_reference operator[](size_type pos) const { return data_[pos]; }

    /// @brief Get pointer to underlying array
    /// @return Pointer to first element
    pointer data() { return data_; }

    /// @brief Get current number of elements
    /// @return Number of elements in array
    kf_nodiscard size_type size() const { return size_; }

    /// @brief Check if array is empty
    /// @return true if array contains no elements
    kf_nodiscard bool empty() const { return size_ == 0; }

    /// @brief Add element to the end of array
    /// @param value Value to copy into array
    /// @note Automatically resizes if capacity is insufficient
    void push_back(const T &value) {
        if (size_ == capacity_) {
            reserve(capacity_ ? capacity_ * 2 : 4);
        }
        allocator_traits<Alloc>::construct(alloc_, data_ + size_, value);
        ++size_;
    }

    /// @brief Remove last element from array
    /// @note Destroys the removed element
    void pop_back() {
        if (size_ > 0) {
            --size_;
            allocator_traits<Alloc>::destroy(alloc_, data_ + size_);
        }
    }

    /// @brief Remove all elements from array
    /// @note Destroys all elements but does not deallocate memory
    void clear() {
        for (usize i = 0; i < size_; ++i) {
            allocator_traits<Alloc>::destroy(alloc_, data_ + i);
        }
        size_ = 0;
    }

    /// @brief Reserve memory for at least new_capacity elements
    /// @param new_capacity Minimum new capacity
    /// @note Reallocates if new_capacity > current capacity
    void reserve(size_type new_capacity) {
        if (new_capacity <= capacity_) { return; }

        T *new_data = alloc_.allocate(new_capacity);

        for (usize i = 0; i < size_; ++i) {
            allocator_traits<Alloc>::construct(alloc_, new_data + i, kf::move(data_[i]));
            allocator_traits<Alloc>::destroy(alloc_, data_ + i);
        }

        if (data_) {
            alloc_.deallocate(data_, capacity_);
        }

        data_ = new_data;
        capacity_ = new_capacity;
    }

    /// @brief Copy constructor is deleted (no copy semantics)
    ArrayList(const ArrayList &) = delete;

    /// @brief Copy assignment is deleted (no copy semantics)
    ArrayList &operator=(const ArrayList &) = delete;
};

}// namespace kf

#endif
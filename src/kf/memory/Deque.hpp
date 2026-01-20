// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/config.hpp"
#include "kf/memory/Allocator.hpp"

#if kf_port_has_deque

#include <deque>

namespace kf {

template<typename T, typename Alloc = kf::Allocator<T>> using Deque = std::deque<T, Alloc>;

}

#else

#include "kf/algorithm.hpp"
#include "kf/core/aliases.hpp"
#include "kf/core/attributes.hpp"
#include "kf/core/type_traits.hpp"
#include "kf/core/utility.hpp"

namespace kf {

/// @brief Circular buffer-based deque implementation for embedded systems
/// @tparam T Element type
/// @tparam Alloc Allocator type (default: kf::allocator<T>)
/// @note Provides double-ended queue functionality with circular buffer storage
template<typename T, typename Alloc = allocator<T>>
class Deque {
private:
    T *buffer_ = nullptr;///< Circular buffer storage
    usize capacity_ = 0; ///< Total allocated capacity
    usize size_ = 0;     ///< Current number of elements
    usize front_ = 0;    ///< Index of first element in circular buffer
    Alloc alloc_;        ///< Allocator instance

public:
    using value_type = T;                      ///< Element type
    using allocator_type = Alloc;              ///< Allocator type
    using size_type = usize;                   ///< Size type
    using reference = value_type &;            ///< Reference to element
    using const_reference = const value_type &;///< Const reference to element

    /// @brief Default constructor
    Deque() = default;

    /// @brief Construct with custom allocator
    /// @param alloc Allocator instance to use
    explicit Deque(const allocator_type &alloc) :
        alloc_(alloc) {}

    /// @brief Destructor - destroys all elements and deallocates buffer
    ~Deque() {
        clear();
        if (buffer_) {
            alloc_.deallocate(buffer_, capacity_);
        }
    }

    /// @brief Access element without bounds checking
    /// @param pos Element index (0 = front, size-1 = back)
    /// @return Reference to element at index
    reference operator[](size_type pos) {
        return buffer_[(front_ + pos) % capacity_];
    }

    /// @brief Access element without bounds checking (const version)
    /// @param pos Element index (0 = front, size-1 = back)
    /// @return Const reference to element at index
    const_reference operator[](size_type pos) const {
        return buffer_[(front_ + pos) % capacity_];
    }

    /// @brief Access first element
    /// @return Reference to front element
    reference front() { return buffer_[front_]; }

    /// @brief Access first element (const version)
    /// @return Const reference to front element
    const_reference front() const { return buffer_[front_]; }

    /// @brief Access last element
    /// @return Reference to back element
    reference back() { return buffer_[(front_ + size_ - 1) % capacity_]; }

    /// @brief Access last element (const version)
    /// @return Const reference to back element
    const_reference back() const { return buffer_[(front_ + size_ - 1) % capacity_]; }

    /// @brief Check if deque is empty
    /// @return true if deque contains no elements
    kf_nodiscard bool empty() const { return size_ == 0; }

    /// @brief Get current number of elements
    /// @return Number of elements in deque
    kf_nodiscard size_type size() const { return size_; }

    /// @brief Get current allocated capacity
    /// @return Total capacity of internal buffer
    kf_nodiscard size_type capacity() const { return capacity_; }

    /// @brief Add element to the back (copy version)
    /// @param value Value to copy to back of deque
    void push_back(const T &value) {
        if (size_ == capacity_) {
            reserve(capacity_ ? capacity_ * 2 : 4);
        }
        usize pos = (front_ + size_) % capacity_;
        allocator_traits<Alloc>::construct(alloc_, buffer_ + pos, value);
        ++size_;
    }

    /// @brief Add element to the back (move version)
    /// @param value Value to move to back of deque
    void push_back(T &&value) {
        if (size_ == capacity_) {
            reserve(capacity_ ? capacity_ * 2 : 4);
        }
        usize pos = (front_ + size_) % capacity_;
        allocator_traits<Alloc>::construct(alloc_, buffer_ + pos, kf::move(value));
        ++size_;
    }

    /// @brief Add element to the front (copy version)
    /// @param value Value to copy to front of deque
    void push_front(const T &value) {
        if (size_ == capacity_) {
            reserve(capacity_ ? capacity_ * 2 : 4);
        }
        front_ = (front_ == 0) ? capacity_ - 1 : front_ - 1;
        allocator_traits<Alloc>::construct(alloc_, buffer_ + front_, value);
        ++size_;
    }

    /// @brief Add element to the front (move version)
    /// @param value Value to move to front of deque
    void push_front(T &&value) {
        if (size_ == capacity_) {
            reserve(capacity_ ? capacity_ * 2 : 4);
        }
        front_ = (front_ == 0) ? capacity_ - 1 : front_ - 1;
        allocator_traits<Alloc>::construct(alloc_, buffer_ + front_, kf::move(value));
        ++size_;
    }

    /// @brief Remove element from the back
    /// @note Destroys the removed element
    void pop_back() {
        if (size_ > 0) {
            usize pos = (front_ + size_ - 1) % capacity_;
            allocator_traits<Alloc>::destroy(alloc_, buffer_ + pos);
            --size_;
        }
    }

    /// @brief Remove element from the front
    /// @note Destroys the removed element and updates front index
    void pop_front() {
        if (size_ > 0) {
            allocator_traits<Alloc>::destroy(alloc_, buffer_ + front_);
            front_ = (front_ + 1) % capacity_;
            --size_;
        }
    }

    /// @brief Remove all elements from deque
    /// @note Destroys all elements but preserves buffer capacity
    void clear() {
        for (usize i = 0; i < size_; ++i) {
            usize pos = (front_ + i) % capacity_;
            allocator_traits<Alloc>::destroy(alloc_, buffer_ + pos);
        }
        size_ = 0;
        front_ = 0;
    }

    /// @brief Reserve memory for at least new_capacity elements
    /// @param new_capacity Minimum new capacity
    /// @note Reallocates buffer if needed and linearizes element order
    void reserve(size_type new_capacity) {
        if (new_capacity <= capacity_) return;

        T *new_buffer = alloc_.allocate(new_capacity);

        for (usize i = 0; i < size_; ++i) {
            usize old_pos = (front_ + i) % capacity_;
            usize new_pos = i;
            allocator_traits<Alloc>::construct(alloc_, new_buffer + new_pos,
                                               kf::move(buffer_[old_pos]));
            allocator_traits<Alloc>::destroy(alloc_, buffer_ + old_pos);
        }

        if (buffer_) {
            alloc_.deallocate(buffer_, capacity_);
        }

        buffer_ = new_buffer;
        capacity_ = new_capacity;
        front_ = 0;
    }

    /// @brief Copy constructor is deleted (no copy semantics)
    Deque(const Deque &) = delete;

    /// @brief Copy assignment is deleted (no copy semantics)
    Deque &operator=(const Deque &) = delete;
};

}// namespace kf

#endif
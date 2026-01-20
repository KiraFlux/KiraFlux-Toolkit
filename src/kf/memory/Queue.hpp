// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/core/config.hpp"
#include "kf/memory/Deque.hpp"

#if kf_port_has_queue

#include <queue>

namespace kf {

/// @brief FIFO (first-in, first-out) queue adapter
/// @tparam T Element type
/// @tparam Container Underlying container type (default: kf::Deque<T>)
/// @note Wrapper around std::queue for platforms with standard library support
template<typename T, typename Container = kf::Deque<T>> using Queue = std::queue<T, Container>;

}// namespace kf

#else

#include "kf/core/attributes.hpp"

namespace kf {

/// @brief FIFO (first-in, first-out) queue implementation for embedded systems
/// @tparam T Element type
/// @tparam Container Underlying container type (default: kf::Deque<T>)
/// @note Adapter over a container that provides queue operations (push back, pop front)
template<typename T, typename Container = kf::Deque<T>>
class Queue {
private:
    Container c;///< Underlying container

public:
    using value_type = typename Container::value_type;          ///< Element type
    using container_type = Container;                           ///< Container type
    using size_type = typename Container::size_type;            ///< Size type
    using reference = typename Container::reference;            ///< Reference to element
    using const_reference = typename Container::const_reference;///< Const reference to element

    /// @brief Default constructor
    Queue() = default;

    /// @brief Access first element (oldest in queue)
    /// @return Reference to front element
    reference front() { return c.front(); }

    /// @brief Access first element (const version)
    /// @return Const reference to front element
    const_reference front() const { return c.front(); }

    /// @brief Access last element (newest in queue)
    /// @return Reference to back element
    reference back() { return c.back(); }

    /// @brief Access last element (const version)
    /// @return Const reference to back element
    const_reference back() const { return c.back(); }

    /// @brief Check if queue is empty
    /// @return true if queue contains no elements
    kf_nodiscard bool empty() const { return c.empty(); }

    /// @brief Get current number of elements
    /// @return Number of elements in queue
    size_type size() const { return c.size(); }

    /// @brief Insert element at the end (copy version)
    /// @param value Value to copy to back of queue
    void push(const value_type &value) { c.push_back(value); }

    /// @brief Insert element at the end (move version)
    /// @param value Value to move to back of queue
    void push(value_type &&value) { c.push_back(kf::move(value)); }

    /// @brief Remove first element (oldest in queue)
    void pop() { c.pop_front(); }

    /// @brief Swap contents with another queue
    /// @param other Queue to swap with
    void swap(Queue &other) noexcept {
        Container temp = kf::move(c);
        c = kf::move(other.c);
        other.c = kf::move(temp);
    }
};

}// namespace kf

#endif
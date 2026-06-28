// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <new>    // placement new
#include <utility>// move, forward

#include "kf/Option.hpp"
#include "kf/Sequence.hpp"
#include "kf/Slice.hpp"
#include "kf/mixin/Resettable.hpp"
#include "kf/primitives.hpp"

namespace kf {

/// @brief LIFO container on fixed‑size buffer (non‑owning)
/// @tparam T Element type
/// @note Inherits Sequence, so all its methods (iterators, slice, etc.) are available
template<typename T> struct Stack : Sequence<Stack<T>, T>, mixin::Resettable<Stack<T>> {

    /// @brief Construct empty stack (no buffer, length 0)
    constexpr Stack() noexcept :
        _buffer{}, _length{0} {}

    /// @brief Construct stack on existing buffer
    /// @param buffer       Slice of memory to use as storage
    /// @param init_length  Initial number of elements already in buffer (default 0)
    explicit constexpr Stack(Slice<T> buffer, usize init_length = 0) noexcept :
        _buffer{buffer}, _length{init_length} {}

    /// @brief Get Stack capacity (max length)
    [[nodiscard]] constexpr usize capacity() const noexcept {
        return _buffer.length();
    }

    /// @brief Is Stack reach max length
    [[nodiscard]] constexpr bool full() const noexcept {
        return this->length() >= this->capacity();
    }

    /// @brief Get mutable access to stack top value
    /// @return optional reference to stack top item
    [[nodiscard]] constexpr auto top() noexcept -> Option<T &> {
        return this->empty() ? none : someRef(_buffer[_length - 1]);
    }

    /// @brief Get Readonly access to stack top value
    /// @return optional constant reference to stack top item
    [[nodiscard]] constexpr auto top() const noexcept -> Option<const T &> {
        return this->empty() ? none : someRef<const T &>(_buffer[_length - 1]);
    }

    /// @brief Put Item on stack top
    /// @return true if item added, false otherwise
    template<typename U> [[nodiscard]] constexpr bool push(U &&item) noexcept {
        if (this->full()) {
            return false;
        }

        new (&_buffer[_length]) T(std::forward<U>(item));

        _length += 1;

        return true;
    }

    /// @brief Get item from top
    /// @return option with item if some, none if empty
    [[nodiscard]] constexpr Option<T> pop() noexcept {
        if (this->empty()) {
            return none;
        }

        _length -= 1;

        auto value = std::move(_buffer[_length]);
        _buffer[_length].~T();

        return some(std::move(value));
    }

private:
    Slice<T> _buffer;
    usize _length;

    using This = Stack<T>;

    KF_IMPL_SEQUENCE(This, T);

    constexpr T *getDataImpl() noexcept {
        return _buffer.data();
    }

    constexpr usize lengthImpl() const noexcept {
        return _length;
    }

    KF_IMPL_RESETTABLE(This);
    constexpr void resetImpl() noexcept {
        for (auto &item: *this) {
            item.~T();
        }

        _length = 0;
    }
};

}// namespace kf
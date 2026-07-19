// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <new>    // placement new
#include <utility>// move, forward

#include "kf/Option.hpp"
#include "kf/Sequence.hpp"
#include "kf/Slice.hpp"
#include "kf/mixin/Readable.hpp"
#include "kf/mixin/Resettable.hpp"
#include "kf/mixin/Writable.hpp"
#include "kf/primitives.hpp"

namespace kf {

struct StackTag {};

/// @brief LIFO container on fixed‑size buffer (non‑owning)
/// @tparam T Element type
template<typename T> struct Stack :

    StackTag,
    Sequence<Stack<T>, T>,
    mixin::Readable<Stack<T>, T>,
    mixin::Writable<Stack<T>, T>,
    mixin::Resettable<Stack<T>>

{

    using Self = Stack<T>;

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

private:
    Slice<T> _buffer;
    usize _length;

    KF_IMPL_SEQUENCE(Self, T);

    constexpr T *getDataImpl() noexcept {
        return _buffer.data();
    }

    constexpr usize lengthImpl() const noexcept {
        return _length;
    }

    KF_IMPL_READABLE(Self, T);
    constexpr auto readImpl() noexcept -> Option<T> {
        if (this->empty()) {
            return none;
        }

        _length -= 1;

        auto value = std::move(_buffer[_length]);
        _buffer[_length].~T();

        return some(std::move(value));
    }

    KF_IMPL_WRITABLE(Self, T);
    constexpr bool writeImpl(auto &&item) noexcept {
        if (this->full()) {
            return false;
        }

        new (&_buffer[_length]) T(std::forward<decltype(item)>(item));

        _length += 1;

        return true;
    }

    KF_IMPL_RESETTABLE(Self);
    constexpr void resetImpl() noexcept {
        for (auto &item: *this) {
            item.~T();
        }

        _length = 0;
    }
};

}// namespace kf
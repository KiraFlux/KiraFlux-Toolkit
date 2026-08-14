// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    Arena.hpp
/// @brief   Linear allocator with O(1) reset for fast, deterministic memory allocation.

#pragma once

#include <new>    // placement new
#include <utility>// std::forward

#include "kf/Bytes.hpp"
#include "kf/Option.hpp"
#include "kf/Slice.hpp"
#include "kf/core.hpp"

#include "kf/mixin/ExtraAllocationLength.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf {

/// @brief Linear allocator with O(1) reset
/// @note The caller is responsible for calling destructors for all allocated objects
///       before resetting the arena or discarding the memory.
///       Use with container types (like Stack, Queue,  etc..) that manage lifetime automatically.
struct Arena : mixin::NonCopyable, mixin::Resettable<Arena> {
    using Self = Arena;

    /// @param buffer Raw memory for the arena
    explicit constexpr Arena(Bytes buffer) noexcept :
        _buffer{buffer} {}

    /// @return Remaining free bytes
    [[nodiscard]] constexpr usize available() const noexcept {
        return _buffer.length() - _used;
    }

    /// @brief Check if pointer belongs to this arena
    [[nodiscard]] constexpr bool owns(void const *ptr) const noexcept {
        return ptr >= _buffer.data() and ptr < _buffer.data() + _used;
    }

    /// @brief Allocate raw bytes
    /// @return Slice or empty on failure
    [[nodiscard]] constexpr auto allocate(usize size, usize alignment = alignof(std::max_align_t)) noexcept -> Bytes {
        usize const aligned = (_used + alignment - 1) & ~(alignment - 1);

        if (aligned > _buffer.length() - size) {
            return {};
        }

        _used = aligned + size;
        return {_buffer.data() + aligned, size};
    }

    /// @brief Allocate uninitialized storage for objects
    /// @note Caller must call destructor for all active instances (like Stack's reset() do)
    template<typename T> [[nodiscard]] constexpr auto allocate(usize count) noexcept -> Slice<T> {
        auto bytes = allocate(count * sizeof(T), alignof(T));

        return {
            reinterpret_cast<T *>(bytes.data()),
            static_cast<usize>(bytes.length() / sizeof(T)),
        };
    }

    /// @brief Allocate and construct an object
    /// @note Caller must call destructor manually
    template<typename T, typename... Args> auto create(Args &&...args) noexcept -> Option<T &> {
        usize total_size = sizeof(T);

        if constexpr (implements<T, mixin::ExtraAllocationLengthTag>) {
            total_size += T::extraAllocationLength(args...);
        }

        if (available() < total_size) { return none; }

        auto ptr = static_cast<void *>(allocate(sizeof(T), alignof(T)).data());

        if constexpr (implements<T, mixin::ExtraAllocationLengthTag>) {
            new (ptr) T{*this, std::forward<Args>(args)...};
        } else {
            new (ptr) T{std::forward<Args>(args)...};
        }

        return someRef(*static_cast<T *>(ptr));
    }

private:
    Bytes _buffer{};
    usize _used{0};

    KF_IMPL_RESETTABLE(Self);
    constexpr void resetImpl() noexcept {
        _used = 0;
    }
};

}// namespace kf
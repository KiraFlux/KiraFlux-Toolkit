// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <new>    // placement new
#include <utility>// std::forward

#include "kf/Option.hpp"
#include "kf/Slice.hpp"
#include "kf/concepts.hpp"
#include "kf/primitives.hpp"

#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf {

/// @brief Linear allocator with O(1) reset
/// @note Supports only trivial types. Memory is not freed individually
struct Arena : mixin::NonCopyable, mixin::Resettable<Arena> {
    using Self = Arena;

    /// @param buffer Raw memory for the arena
    explicit constexpr Arena(Slice<u8> buffer) noexcept :
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
    [[nodiscard]] constexpr auto allocate(usize size, usize alignment = alignof(std::max_align_t)) noexcept -> Slice<u8> {
        usize const aligned = (_used + alignment - 1) & ~(alignment - 1);

        if (aligned > _buffer.length() - size) {
            return {};
        }

        _used = aligned + size;
        return {_buffer.data() + aligned, size};
    }

    /// @brief Allocate uninitialized storage for trivial objects
    template<trivial T> [[nodiscard]] constexpr auto allocate(usize count) noexcept -> Slice<T> {
        auto bytes = allocate(count * sizeof(T), alignof(T));

        return {
            reinterpret_cast<T *>(bytes.data()),
            static_cast<usize>(bytes.length() / sizeof(T)),
        };
    }

    /// @brief Allocate and construct a trivial object
    template<trivial T, typename... Args> auto create(Args &&...args) noexcept -> Option<T &> {
        auto mem = allocate(sizeof(T), alignof(T));

        if (mem.empty()) {
            return none;
        }

        return someRef(*(new (static_cast<void *>(mem.data())) T{std::forward<Args>(args)...}));
    }

private:
    Slice<u8> _buffer{};
    usize _used{0};

    KF_IMPL_RESETTABLE(Self);
    constexpr void resetImpl() noexcept {
        _used = 0;
    }
};

}// namespace kf
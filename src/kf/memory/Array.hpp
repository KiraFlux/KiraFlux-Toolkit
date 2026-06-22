// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <array>

#include "kf/Slice.hpp"
#include "kf/primitives.hpp"

namespace kf::memory {

/// @brief Array. Owns items
/// @note Wrapper for Array from STL
/// @tparam T Item type
/// @tparam N Items total
template<typename T, usize N> struct Array : std::array<T, N> {

    /// @brief Get mutable slice
    [[nodiscard]] constexpr Slice<T> slice() noexcept {
        return {
            this->data(),
            this->size(),
        };
    }

    /// @brief Get readonly slice
    [[nodiscard]] constexpr Slice<const T> slice() const noexcept {
        return {
            this->data(),
            this->size(),
        };
    }
};

}// namespace kf::memory
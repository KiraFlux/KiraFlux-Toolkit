// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/primitives.hpp"

namespace kf::mixin {

struct IndexableTag {};

/// @brief Adds Index operator
/// @tparam Impl Indexable implementation class with `constexpr T getItemImpl(usize index) noexcept` method
/// @tparam T item type
template<typename Impl, typename T> struct Indexable : IndexableTag {

    /// @brief Get mutable access to item by index
    [[nodiscard]] constexpr T &operator[](usize index) noexcept {
        return static_cast<Impl *>(this)->getItemImpl(index);
    }

    /// @brief Get readonly access to item by index
    [[nodiscard]] constexpr const T &operator[](usize index) const noexcept {
        return (*const_cast<Indexable *>(this))[index];
    }
};

}// namespace kf::mixin

#define KF_IMPL_INDEXABLE(__impl__, __type__) friend struct ::kf::mixin::Indexable<__impl__, __type__>
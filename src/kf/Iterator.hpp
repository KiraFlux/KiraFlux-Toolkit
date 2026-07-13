// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Equatable.hpp"

namespace kf {

struct IteratorTag {};

/// @brief CRTP base for custom iterators
/// @tparam Impl Implementation class
/// @tparam T   Element type (value or reference)
template<typename Impl, typename T> struct Iterator : IteratorTag, mixin::Equatable<Impl> {

    /// @brief Get current element
    [[nodiscard]] constexpr T value() const noexcept {
        return static_cast<const Impl *>(this)->getValueImpl();
    }

    /// @brief Advance to next element
    Impl &next() noexcept {
        static_cast<Impl *>(this)->nextImpl();
        return *static_cast<Impl *>(this);
    }

    // C++ Iterator methods

    /// @brief Dereference: value()
    [[nodiscard]] constexpr T operator*() const noexcept {
        return value();
    }

    /// @brief Prefix increment: next()
    Impl &operator++() noexcept {
        return next();
    }
};

}// namespace kf

#define KF_IMPL_ITERATOR(__impl__, ...)                  \
    friend struct ::kf::Iterator<__impl__, __VA_ARGS__>; \
    KF_IMPL_EQUATABLE(__impl__)

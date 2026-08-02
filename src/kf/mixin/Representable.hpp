// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/Representable.hpp
/// @brief   Adds repr() method for string representation.

#pragma once

namespace kf::mixin {

struct RepresentableTag {};

/// @brief CRTP mixin that adds a `repr()` method
/// @tparam Impl       Implementation class (must implement `ReprType reprImpl() const noexcept`)
/// @tparam ReprType Return type of `repr()`
template<typename Impl, typename ReprType> struct Representable : RepresentableTag {

    /// @brief Get string representation of the object
    /// @return String of type `ReprType`
    [[nodiscard]] constexpr ReprType repr() const noexcept {
        return static_cast<Impl const *>(this)->reprImpl();
    }
};

}// namespace kf::mixin

#define KF_IMPL_REPRESENTABLE(...) friend struct ::kf::mixin::Representable<__VA_ARGS__>

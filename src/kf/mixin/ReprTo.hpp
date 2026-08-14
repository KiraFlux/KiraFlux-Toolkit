// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/ReprTo.hpp
/// @brief   Adds reprTo() method for string representation.

#pragma once

#include "kf/core.hpp"

namespace kf::mixin {

template<typename> struct WritableTag;

struct ReprToTag {};

/// @brief CRTP mixin that adds a `reprTo(char_writable)` method
/// @tparam Impl Implementation class (must implement `ReprType reprToImpl() const noexcept`)
template<typename Impl> struct ReprTo : ReprToTag {

    /// @brief Represent object to the writable
    constexpr void reprTo(implements<mixin::WritableTag<char>> auto &char_writable) const noexcept {
        static_cast<Impl const *>(this)->reprToImpl(char_writable);
    }
};

}// namespace kf::mixin

#define KF_IMPL_REPR_TO(...) friend struct ::kf::mixin::ReprTo<__VA_ARGS__>

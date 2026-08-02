// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    filter/Filter.hpp
/// @brief   CRTP base for single‑sample filters (calc()).

#pragma once

#include "kf/concepts.hpp"

#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf::filter {

struct FilterTag {};

/// @brief Filter static interface
/// @tparam Impl Derived filter implementation
/// @tparam T    Input/output type
template<typename Impl, arithmetic T> struct Filter :

    FilterTag,
    mixin::NonCopyable,
    mixin::Resettable<Impl>

{

    /// @brief Updates the filter with a new sample.
    /// @param value The new input value.
    /// @return The filtered value after the update.
    [[nodiscard]] T calc(T const &value) noexcept {
        return static_cast<Impl *>(this)->calcImpl(value);
    }
};

}// namespace kf::filter

#define KF_IMPL_FILTER(__impl__, ...)                          \
    friend struct ::kf::filter::Filter<__impl__, __VA_ARGS__>; \
    KF_IMPL_RESETTABLE(__impl__)

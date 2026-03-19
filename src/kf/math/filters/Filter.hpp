// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf::math::filters {

struct FilterTag {};

/// @brief CRTP mixin reset.
/// @tparam Impl Derived class (must provide `resetImpl()`).
template<typename Impl, typename T> struct Filter : FilterTag, mixin::NonCopyable, mixin::Resettable<Impl> {

    /// @brief Update filter with new sample
    /// @param value New input value
    /// @return Current filtered value after update
    [[nodiscard]] T calc(const T &value) noexcept { return static_cast<Impl *>(this)->calcImpl(value); }
};

}// namespace kf::math::filters

#define KF_IMPL_FILTER(__impl__, __value_type__) friend struct kf::math::filters::Filter<__impl__, __value_type__>
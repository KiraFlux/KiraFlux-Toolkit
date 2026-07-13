// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/meta/CRTP.hpp"
#include "kf/mixin/NonCopyable.hpp"
#include "kf/mixin/Resettable.hpp"

namespace kf::filter {

struct FilterTag {};

/// @brief CRTP mixin reset.
/// @tparam Impl Derived filter implementation.
/// @tparam T    Input/output type.
template<typename Impl, typename T> struct Filter : FilterTag, mixin::NonCopyable, mixin::Resettable<Impl>, meta::CRTP<Impl> {

    /// @brief Updates the filter with a new sample.
    /// @param value The new input value.
    /// @return The filtered value after the update.
    [[nodiscard]] T calc(const T &value) noexcept {
        return this->impl().calcImpl(value);
    }
};

}// namespace kf::filter

#define KF_IMPL_FILTER(__impl__, ...)                          \
    friend struct ::kf::filter::Filter<__impl__, __VA_ARGS__>; \
    KF_IMPL_RESETTABLE(__impl__)

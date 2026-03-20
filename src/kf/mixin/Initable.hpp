// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

namespace kf::mixin {

struct InitableTag {};

/// @brief CRTP mixin initialisation.
/// @tparam Impl Derived class (must provide `initImpl()`).
/// @tparam T    Return type of init() (void or any).
template<typename Impl, typename T> struct Initable : InitableTag {
    [[nodiscard]] T init() noexcept {
        return static_cast<Impl *>(this)->initImpl();
    }
};

/// @brief Void specialisation.
template<typename Impl> struct Initable<Impl, void> : InitableTag {
    void init() noexcept {
        static_cast<Impl *>(this)->initImpl();
    }
};

}// namespace kf::mixin

#define KF_IMPL_INITABLE(__impl__, __init_result_type) friend struct ::kf::mixin::Initable<__impl__, __init_result_type>
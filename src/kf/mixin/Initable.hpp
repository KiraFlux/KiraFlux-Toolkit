// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>

#include "kf/meta/CRTP.hpp"

namespace kf::mixin {

struct InitableTag {};

/// @brief CRTP mixin initialisation.
/// @tparam Impl Derived class (must provide `initImpl()`).
/// @tparam T    Return type of init() (void or any).
template<typename Impl, typename T, typename = void> struct Initable : InitableTag {
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
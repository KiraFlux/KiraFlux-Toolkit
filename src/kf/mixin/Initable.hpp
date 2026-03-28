// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf::mixin {

struct InitableTag {};

/// @brief CRTP mixin initialisation.
/// @tparam Impl The derived class that must implement `initImpl()`.
/// @tparam T   Return type of the initialization (void or some Result/error type).
template<typename Impl, typename T> struct Initable : InitableTag {

    /// @tparam T   Return type of the initialization (void or some Result/error type).
    [[nodiscard]] T init() noexcept {
        return static_cast<Impl *>(this)->initImpl();
    }
};

/// @brief Void specialisation.
template<typename Impl> struct Initable<Impl, void> : InitableTag {

    /// @brief Initializes the object.
    void init() noexcept {
        static_cast<Impl *>(this)->initImpl();
    }
};

}// namespace kf::mixin

#define KF_IMPL_INITABLE(__impl__, __init_result_type) friend struct ::kf::mixin::Initable<__impl__, __init_result_type>
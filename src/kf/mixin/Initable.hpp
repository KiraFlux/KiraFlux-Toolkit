// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <utility>

namespace kf::mixin {

struct InitableTag {};

/// @brief CRTP mixin that adds an `init(Args...)` method forwarding to `initImpl(Args...)`
/// @tparam Impl       Implementation class (must implement `initImpl(Args...)`)
/// @tparam Signature  Function signature `R(Args...)` - return type and arguments
template<typename Impl, typename Signature> struct Initable;

template<typename Impl, typename... Args> struct Initable<Impl, void(Args...)> : InitableTag {

    /// @brief Initializes the object with given arguments
    /// @param args Arguments forwarded to `initImpl`
    void init(Args... args) noexcept {
        static_cast<Impl *>(this)->initImpl(std::forward<Args>(args)...);
    }
};

template<typename Impl, typename R, typename... Args> struct Initable<Impl, R(Args...)> : InitableTag {

    /// @brief Initializes the object with given arguments
    /// @param args Arguments forwarded to `initImpl`
    /// @return Value returned by `initImpl`
    [[nodiscard]] R init(Args... args) noexcept {
        return static_cast<Impl *>(this)->initImpl(std::forward<Args>(args)...);
    }
};

}// namespace kf::mixin

#define KF_IMPL_INITABLE(...) friend struct ::kf::mixin::Initable<__VA_ARGS__>

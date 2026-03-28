// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/NonCopyable.hpp"

namespace kf::mixin {

/// @brief Thread-safe Singleton pattern implementation
/// @tparam Impl Type to be instantiated as singleton
/// @note Uses Meyer's singleton (static local variable) for thread safety
template<typename Impl> struct Singleton : kf::mixin::NonCopyable {
    /// @brief Get reference to the single instance
    /// @return Reference to the unique instance of type Impl
    /// @note Instance is created on first call (lazy initialization)
    static Impl &instance() noexcept {
        static Impl instance{};// NOLINT(*-dynamic-static-initializers)
        return instance;
    }

protected:
    Singleton() = default;
    ~Singleton() = default;
};

}// namespace kf::mixin
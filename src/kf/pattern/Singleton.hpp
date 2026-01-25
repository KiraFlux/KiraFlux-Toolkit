// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

namespace kf {

/// @brief Thread-safe Singleton pattern implementation
/// @tparam T Type to be instantiated as singleton
/// @note Uses Meyer's singleton (static local variable) for thread safety
template<typename T> struct Singleton {

    /// @brief Get reference to the single instance
    /// @return Reference to the unique instance of type T
    /// @note Instance is created on first call (lazy initialization)
    static T &instance() noexcept {
        static T instance{};// NOLINT(*-dynamic-static-initializers)
        return instance;
    }

    /// @brief Deleted copy constructor (non-copyable)
    Singleton(const Singleton &) = delete;

    /// @brief Deleted copy assignment operator (non-copyable)
    Singleton &operator=(const Singleton &) = delete;

protected:
    /// @brief Protected default constructor (ensures single instance)
    Singleton() = default;

    /// @brief Protected destructor (ensures controlled lifetime)
    ~Singleton() = default;
};

}// namespace kf
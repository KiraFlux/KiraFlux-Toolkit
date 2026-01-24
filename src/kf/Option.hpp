// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdlib>

#include "kf/core/attributes.hpp"


namespace kf {

/// @brief Optional value container (similar to std::optional)
/// @tparam T Value type (must be trivially copyable and destructible)
/// @note Embedded-friendly implementation without exceptions or heap allocation
template<typename T> struct Option {

private:
    union {
        T val;     ///< Storage for value when engaged
        char dummy;///< Dummy member for empty state
    };
    bool engaged;///< Flag indicating whether value is present

public:
    /// @brief Construct Option with value (copy)
    /// @param value Value to store in Option
    constexpr Option(const T &value) :// NOLINT(*-explicit-constructor)
        engaged{true}, val{value} {}

    /// @brief Construct empty Option (no value)
    constexpr Option() :
        engaged{false}, dummy{0} {}

    /// @brief Check if Option contains a value
    /// @return true if value is present, false otherwise
    kf_nodiscard bool hasValue() const { return engaged; }

    /// @brief Get stored value (unsafe)
    /// @return Reference to stored value
    /// @warning Causes abort() if Option is empty
    /// @note Use hasValue() to check before calling
    kf_nodiscard T &value() {
        if (engaged) {
            return val;
        } else {
            abort();
        }
    }

    /// @brief Get stored value or default
    /// @param default_value Value to return if Option is empty
    /// @return Stored value if present, default_value otherwise
    /// @note Safe alternative to value() that doesn't terminate
    kf_nodiscard T valueOr(const T &default_value) const {
        return engaged ? val : default_value;
    }
};

}// namespace kf
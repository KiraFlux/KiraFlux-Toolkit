// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdlib>

namespace kf {

/// @brief Optional value container (similar to std::optional)
/// @tparam T Value type (must be trivially copyable and destructible)
/// @note Embedded-friendly implementation without exceptions or heap allocation
template<typename T> struct Option {
    static_assert(std::is_trivially_destructible_v<T>, "T must be trivially destructible");

private:
    union {
        T _value;   ///< Storage for value when engaged
        char _dummy;///< Dummy member for empty state
    };
    bool _engaged;///< Flag indicating whether value is present

public:
    /// @brief Construct Option with value (copy)
    /// @param value Value to store in Option
    constexpr Option(const T &value) noexcept :// NOLINT(*-explicit-constructor)
        _engaged{true}, _value{value} {}

    /// @brief Construct empty Option (no value)
    constexpr Option() noexcept :
        _engaged{false}, _dummy{0} {}

    /// @brief Check if Option contains a value
    /// @return true if value is present, false otherwise
    [[nodiscard]] constexpr bool hasValue() const noexcept { return _engaged; }

    /// @brief Get stored value (unsafe)
    /// @return Reference to stored value
    /// @warning Causes abort() if Option is empty
    /// @note Use hasValue() to check before calling
    [[nodiscard]] T &value() noexcept {
        if (_engaged) {
            return _value;
        } else {
            abort();
        }
    }

    [[nodiscard]] const T &value() const noexcept {
        if (_engaged) {
            return _value;
        } else {
            abort();
        }
    }

    void value(const T& new_value) noexcept {
        _engaged = true;
        _value = new_value;
    }

    /// @brief Get stored value or default
    /// @param default_value Value to return if Option is empty
    /// @return Stored value if present, default_value otherwise
    /// @note Safe alternative to value() that doesn't terminate
    [[nodiscard]] constexpr T valueOr(const T &default_value) const noexcept {
        return _engaged ? _value : default_value;
    }
};

}// namespace kf
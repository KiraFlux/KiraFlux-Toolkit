// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility>

#include "kf/mixin/Resettable.hpp"

namespace kf {

/// @brief Optional value container (similar to std::optional)
/// @tparam T Value type (must be trivially copyable and destructible)
/// @note Embedded-friendly implementation without exceptions or heap allocation
template<typename T> struct Option final : mixin::Resettable<Option<T>> {
    static_assert(std::is_trivially_destructible_v<T>, "T must be trivially destructible");
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");

    /// @brief Construct Option with value (copy)
    constexpr Option(const T &value) noexcept :// NOLINT(*-explicit-constructor)
        _is_some{true}, _value{value} {}

    /// @brief Construct Option with value (move)
    constexpr Option(T &&value) noexcept :// NOLINT(*-explicit-constructor)
        _is_some{true}, _value{std::move(value)} {}

    /// @brief Construct empty Option (no value)
    constexpr Option() noexcept :
        _is_some{false}, _dummy{0} {}

    /// @brief Check if Option contains a value
    /// @return true if value is present, false otherwise
    [[nodiscard]] constexpr bool isSome() const noexcept { return _is_some; }

    /// @brief Check if Option does not contains a value
    /// @return true if no value is present, false otherwise
    [[nodiscard]] constexpr bool isNone() const noexcept { return not _is_some; }

    /// @brief Get stored value (unsafe)
    /// @return Reference to stored value
    /// @warning Causes abort() if Option is empty
    /// @note Use isSome() to check before calling
    [[nodiscard]] T &value() noexcept {
        if (_is_some) {
            return _value;
        } else {
            abort();
        }
    }

    [[nodiscard]] const T &value() const noexcept {
        return const_cast<Option *>(this)->value();
    }

    void value(const T &new_value) noexcept {
        _is_some = true;
        _value = new_value;
    }

    void value(T &&new_value) noexcept {
        _is_some = true;
        _value = std::move(new_value);
    }

    /// @brief Get stored value or default
    /// @param default_value Value to return if Option is empty
    /// @return Stored value if present, default_value otherwise
    /// @note Safe alternative to value() that doesn't terminate
    [[nodiscard]] constexpr T valueOr(const T &default_value) const noexcept {
        return _is_some ? _value : default_value;
    }

    [[nodiscard]] constexpr T valueOr(T &&default_value) const noexcept {
        return _is_some ? _value : std::move(default_value);
    }

    /// @brief Transform Option's value if engaged
    /// @tparam _F auto-deducted mapping function type
    /// @param f mapping function
    /// @return A new Option<U> with result of mapping if some, empty option if is none
    template<typename _F> [[nodiscard]] auto map(_F &&f) const noexcept -> Option<decltype(f(std::declval<T>()))> {
        if (isSome()) {
            return {f(value())};
        } else {
            return {};
        }
    }

private:
    union {
        T _value;   ///< Storage for value when engaged
        char _dummy;///< Dummy member for empty state
    };
    bool _is_some;///< Flag indicating whether value is present

    //impl
    using This = Option<T>;

    KF_IMPL_RESETTABLE(This);
    void resetImpl() noexcept { _is_some = false; }
};

}// namespace kf
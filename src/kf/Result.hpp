// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>
#include <utility>

namespace kf {

/// @brief Result type that can hold either a value or an error
/// @tparam T Type of successful result value
/// @tparam E Type of error value
/// @note Embedded-friendly alternative to exceptions for error handling
template<typename T, typename E> struct Result {
    static_assert(not std::is_same_v<T, E>, "T and E must be different types");
    static_assert(std::is_trivially_destructible_v<T>, "T must be trivially destructible");
    static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable");
    static_assert(std::is_trivially_destructible_v<E>, "E must be trivially destructible");
    static_assert(std::is_trivially_copyable_v<E>, "E must be trivially copyable");

    /// @brief Construct successful result with value (move)
    constexpr Result(T &&value) noexcept :
        _is_ok{true}, _value{std::move(value)} {}

    /// @brief Construct successful result with value (copy)
    constexpr Result(const T &value) noexcept :
        _is_ok{true}, _value{value} {}

    /// @brief Construct error result with error (move)
    constexpr Result(E &&error) noexcept :
        _is_ok{false}, _error{std::move(error)} {}

    /// @brief Construct error result with error (copy)
    constexpr Result(const E &error) noexcept :
        _is_ok{false}, _error{error} {}

    /// @brief Check if result contains a value (success)
    [[nodiscard]] constexpr bool isOk() const noexcept { return _is_ok; }

    /// @brief Check if result contains an error
    [[nodiscard]] constexpr bool isError() const noexcept { return not _is_ok; }

    [[nodiscard]] T &value() noexcept {
        if (isOk()) { return _value; }
        abort();
    }

    [[nodiscard]] const T &value() const noexcept {
        if (isOk()) { return _value; }
        abort();
    }

    [[nodiscard]] E &error() noexcept {
        if (isError()) { return _error; }
        abort();
    }

    [[nodiscard]] const E &error() const noexcept {
        if (isError()) { return _error; }
        abort();
    }

private:
    bool _is_ok;///< Flag indicating success (true) or error (false)

    union {
        T _value;///< Storage for successful result (active when is_ok == true)
        E _error;///< Storage for error result (active when is_ok == false)
    };
};

/// @brief Result specialization for void (success-only) operations
/// @tparam E Type of error value
/// @note Used for operations that don't return a value on success
template<typename E> struct Result<void, E> {
    static_assert(std::is_trivially_destructible_v<E>, "E must be trivially destructible");
    static_assert(std::is_trivially_copyable_v<E>, "E must be trivially copyable");

    /// @brief Construct successful void result
    constexpr Result() noexcept :
        _is_ok{true} {}

    /// @brief Construct error result with error (move)
    constexpr Result(E &&error) noexcept :
        _is_ok{false}, _error{std::move(error)} {}

    /// @brief Construct error result with error (copy)
    constexpr Result(const E &error) noexcept :
        _is_ok{false}, _error{error} {}

    /// @brief Check if result is successful
    [[nodiscard]] constexpr bool isOk() const noexcept { return _is_ok; }

    /// @brief Check if result contains an error
    [[nodiscard]] constexpr bool isError() const noexcept { return not _is_ok; }

    [[nodiscard]] E &error() noexcept {
        if (isError()) { return _error; }
        abort();
    }

    [[nodiscard]] const E &error() const noexcept {
        if (isError()) { return _error; }
        abort();
    }

private:
    bool _is_ok;///< Flag indicating success (true) or error (false)
    E _error;   ///< Storage for error result (active when is_ok == false)
};

}// namespace kf
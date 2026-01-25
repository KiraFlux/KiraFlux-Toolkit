// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Option.hpp"
#include "kf/core/attributes.hpp"


namespace kf {

/// @brief Result type that can hold either a value or an error
/// @tparam T Type of successful result value
/// @tparam E Type of error value
/// @note Embedded-friendly alternative to exceptions for error handling
template<typename T, typename E> struct Result {

private:
    bool is_ok;///< Flag indicating success (true) or error (false)

    union {
        T value;///< Storage for successful result (active when is_ok == true)
        E err;  ///< Storage for error result (active when is_ok == false)
    };

public:
    /// @brief Construct successful result with value
    /// @param val Value to store as successful result
    constexpr Result(T val) noexcept:// NOLINT(*-explicit-constructor)
        is_ok{true}, value{val} {}

    /// @brief Construct error result with error
    /// @param error Error value to store
    constexpr Result(E error) noexcept:// NOLINT(*-explicit-constructor)
        is_ok{false}, err{error} {}

    /// @brief Check if result contains a value (success)
    /// @return true if result is successful (contains value)
    kf_nodiscard bool isOk() const noexcept { return is_ok; }

    /// @brief Check if result contains an error
    /// @return true if result contains an error
    kf_nodiscard bool isError() const noexcept { return not is_ok; }

    /// @brief Get successful value as Option
    /// @return Option containing value if successful, empty Option otherwise
    Option<T> ok() const noexcept {
        if (is_ok) {
            return {value};
        } else {
            return {};
        }
    }

    /// @brief Get error value as Option
    /// @return Option containing error if failed, empty Option otherwise
    Option<E> error() const noexcept {
        if (is_ok) {
            return {};
        } else {
            return {err};
        }
    }
};

/// @brief Result specialization for void (success-only) operations
/// @tparam E Type of error value
/// @note Used for operations that don't return a value on success
template<typename E> struct Result<void, E> {

private:
    bool is_ok;///< Flag indicating success (true) or error (false)
    E err;     ///< Storage for error result (active when is_ok == false)

public:
    /// @brief Construct successful void result
    constexpr Result() noexcept:
        is_ok{true} {}

    /// @brief Construct error result with error
    /// @param error Error value to store
    constexpr Result(E error) noexcept:// NOLINT(*-explicit-constructor)
        is_ok{false}, err{error} {}

    /// @brief Check if result is successful
    /// @return true if operation succeeded (no error)
    kf_nodiscard bool isOk() const noexcept { return is_ok; }

    /// @brief Check if result contains an error
    /// @return true if operation failed (contains error)
    kf_nodiscard bool isError() const noexcept { return not is_ok; }

    /// @brief Get error value as Option
    /// @return Option containing error if failed, empty Option otherwise
    Option<E> error() const noexcept {
        if (is_ok) {
            return {};
        } else {
            return {err};
        }
    }
};

}// namespace kf
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility>

namespace kf {

namespace internal {

template<typename Impl, typename E> struct ResultErrorController {
    static_assert(std::is_trivially_destructible_v<E>);
    static_assert(std::is_trivially_copyable_v<E>);

    /// @brief Check if result contains an error
    [[nodiscard]] constexpr bool isError() const noexcept {
        return static_cast<const Impl *>(this)->isErrorImpl();
    }

    /// @brief Get Result error
    [[nodiscard]] const E &error() const noexcept {
        if (isError()) { return const_cast<ResultErrorController *>(this)->getError(); }
        abort();
    }

    /// @brief Set error value via Copy
    void error(const E &new_error) noexcept {
        isError(true);
        getError() = new_error;
    }

    /// @brief Set error value via Move
    void error(E &&new_error) noexcept {
        isError(true);
        getError() = std::move(new_error);
    }

protected:
    void isError(bool is_error) noexcept { return static_cast<Impl *>(this)->isErrorImpl(is_error); }

    E &getError() noexcept { return static_cast<Impl *>(this)->getErrorImpl(); }
};

}// namespace internal

/// @brief Result type that can hold either a value or an error
/// @tparam T Type of successful result value
/// @tparam E Type of error value
/// @note Embedded-friendly alternative to exceptions for error handling
template<typename T, typename E> struct Result final : internal::ResultErrorController<Result<T, E>, E> {
    static_assert(not std::is_same_v<T, E>);
    static_assert(std::is_trivially_destructible_v<T>);
    static_assert(std::is_trivially_copyable_v<T>);

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

    /// @brief Get Result value
    [[nodiscard]] T &value() noexcept {
        if (isOk()) { return _value; }
        abort();
    }

    [[nodiscard]] const T &value() const noexcept { return const_cast<Result *>(this)->value(); }

    void value(T &&new_value) noexcept {
        _is_ok = true;
        _value = std::move(new_value);
    }

    void value(const T &new_value) noexcept {
        _is_ok = true;
        _value = new_value;
    }

private:
    union {
        T _value;///< Storage for successful result (active when is_ok == true)
        E _error;///< Storage for error result (active when is_ok == false)
    };
    bool _is_ok;///< Flag indicating success (true) or error (false)

    friend struct ::kf::internal::ResultErrorController<Result<T, E>, E>;

    [[nodiscard]] bool isErrorImpl() const noexcept { return not _is_ok; }

    void isErrorImpl(bool is_error) noexcept { _is_ok = not is_error; }

    [[nodiscard]] E &getErrorImpl() noexcept { return _error; }
};

/// @brief Result specialization for void (success-only) operations
/// @tparam E Type of error value
/// @note Used for operations that don't return a value on success
template<typename E> struct Result<void, E> final : internal::ResultErrorController<Result<void, E>, E> {

    /// @brief Construct successful void result
    constexpr Result() noexcept :
        _is_error{false} {}

    /// @brief Construct error result with error (move)
    constexpr Result(E &&error) noexcept :
        _is_error{true}, _error{std::move(error)} {}

    /// @brief Construct error result with error (copy)
    constexpr Result(const E &error) noexcept :
        _is_error{true}, _error{error} {}

    /// @brief Check if result is successful
    [[nodiscard]] constexpr bool isOk() const noexcept { return not _is_error; }

    /// @brief Check if result contains an error
    [[nodiscard]] constexpr bool isError() const noexcept { return _is_error; }

    [[nodiscard]] E &error() noexcept {
        if (isError()) { return _error; }
        abort();
    }

    [[nodiscard]] const E &error() const noexcept { return const_cast<Result<void, E> *>(this)->error(); }

private:
    E _error;      ///< Storage for error result
    bool _is_error;///< Flag indicating success error (true) or (false)

    friend struct ::kf::internal::ResultErrorController<Result<void, E>, E>;

    [[nodiscard]] bool isErrorImpl() const noexcept { return _is_error; }

    void isErrorImpl(bool is_error) noexcept { _is_error = is_error; }

    [[nodiscard]] E &getErrorImpl() noexcept { return _error; }
};

}// namespace kf
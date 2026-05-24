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
    constexpr Result(T &&ok) noexcept :
        _is_ok{true}, _ok{std::move(ok)} {}

    /// @brief Construct successful result with value (copy)
    constexpr Result(const T &ok) noexcept :
        _is_ok{true}, _ok{ok} {}

    /// @brief Construct error result with error (move)
    constexpr Result(E &&error) noexcept :
        _is_ok{false}, _error{std::move(error)} {}

    /// @brief Construct error result with error (copy)
    constexpr Result(const E &error) noexcept :
        _is_ok{false}, _error{error} {}

    /// @brief Check if result contains a value (success)
    [[nodiscard]] constexpr bool isOk() const noexcept { return _is_ok; }

    /// @brief Get Result value
    [[nodiscard]] T &ok() noexcept {
        if (isOk()) { return _ok; }
        abort();
    }

    [[nodiscard]] const T &ok() const noexcept { return const_cast<Result *>(this)->ok(); }

    void ok(T &&new_value) noexcept {
        _is_ok = true;
        _ok = std::move(new_value);
    }

    void ok(const T &new_value) noexcept {
        _is_ok = true;
        _ok = new_value;
    }

    /// @brief Transform Result ok
    /// @tparam _F auto-deducted mapping function type
    /// @param f mapping function
    /// @return A new Result<U, E> with mapped value if ok or new result with same Error
    template<typename _F> [[nodiscard]] auto map(_F &&f) const noexcept -> Result<decltype(f(std::declval<T>())), E> {
        if (_is_ok) {
            return {f(_ok)};
        } else {
            return {_error};
        }
    }

    /// @brief Transform Result error
    /// @tparam _F auto-deducted mapping function type
    /// @param f mapping function
    /// @return A new Result<T, W> with mapped error if is error or new result with same value
    template<typename _F> [[nodiscard]] auto mapError(_F &&f) const noexcept -> Result<T, decltype(f(std::declval<E>()))> {
        if (_is_ok) {
            return {_ok};
        } else {
            return {f(_error)};
        }
    }

private:
    union {
        T _ok;   ///< Storage for successful result (active when is_ok == true)
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

    /// @brief Transform Result error
    /// @tparam _F auto-deducted mapping function type
    /// @param f mapping function
    /// @return A new Result<void, W> with mapped error if is error or new result
    template<typename _F> [[nodiscard]] auto mapError(_F &&f) const noexcept -> Result<void, decltype(f(std::declval<E>()))> {
        if (_is_error) {
            return {f(_error)};
        } else {
            return {};
        }
    }

private:
    E _error;      ///< Storage for error result
    bool _is_error;///< Flag indicating success error (true) or (false)

    friend struct ::kf::internal::ResultErrorController<Result<void, E>, E>;

    [[nodiscard]] bool isErrorImpl() const noexcept { return _is_error; }

    void isErrorImpl(bool is_error) noexcept { _is_error = is_error; }

    [[nodiscard]] E &getErrorImpl() noexcept { return _error; }
};

}// namespace kf
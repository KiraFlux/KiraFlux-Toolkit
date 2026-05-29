// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility>

namespace kf {

namespace internal {

/// @brief Base class for result error handling logic (CRTP)
template<typename Impl, typename E> struct ResultErrorController {
    static_assert(std::is_trivially_destructible_v<E>);
    static_assert(std::is_trivially_copyable_v<E>);

    /// @brief Check if result contains an error
    [[nodiscard]] constexpr bool isError() const noexcept {
        return static_cast<const Impl *>(this)->isErrorImpl();
    }

    /// @brief Get the stored error (panic if not error)
    [[nodiscard]] const E &error() const noexcept {
        if (isError()) { return static_cast<const Impl *>(this)->getErrorImpl(); }
        abort();
    }
};

/// @brief Internal wrapper for a success value (used with kf::ok)
template<typename T> struct OkWrapper {
    T value;
};

/// @brief Specialisation of OkWrapper for void
template<> struct OkWrapper<void> {};

/// @brief Internal wrapper for an error value (used with kf::error)
template<typename E> struct ErrorWrapper {
    E value;
};

}// namespace internal

/// @brief Result type that can hold either a value or an error
/// @tparam T Type of successful result value
/// @tparam E Type of error value
/// @note Embedded‑friendly alternative to exceptions.
///       Use kf::ok() and kf::error() to create instances.
/// @see kf::ok, kf::error
template<typename T, typename E> struct Result final : internal::ResultErrorController<Result<T, E>, E> {

    /// @brief Construct from OkWrapper
    /// @see kf::ok(T)
    constexpr Result(internal::OkWrapper<T> ok) noexcept :
        _is_ok{true}, _ok{std::move(ok.value)} {}

    /// @brief Construct from ErrorWrapper
    /// @see kf::error(E)
    constexpr Result(internal::ErrorWrapper<E> error) noexcept :
        _is_ok{false}, _error{std::move(error.value)} {}

    ~Result() noexcept {
        if (_is_ok) {
            _ok.~T();
        } else {
            _error.~E();
        }
    }

    /// @brief Check if result contains a value (success)
    [[nodiscard]] constexpr bool isOk() const noexcept { return _is_ok; }

    /// @brief Get the stored value (undefined behaviour if not ok)
    [[nodiscard]] T &ok() & noexcept {
        if (isOk()) { return _ok; }
        abort();
    }

    [[nodiscard]] T ok() && noexcept {
        if (isOk()) { return std::move(_ok); }
        abort();
    }

    /// @brief Get the stored value (const overload)
    [[nodiscard]] const T &ok() const & noexcept { return const_cast<Result *>(this)->ok(); }

    /// @brief Transform the success value using a function
    /// @tparam F Callable type (auto‑deduced)
    /// @param f Mapping function: T -> U
    /// @return Result<U, E> containing the mapped value on success,
    ///         otherwise the original error.
    template<typename F> [[nodiscard]] auto map(F &&f) const noexcept -> Result<decltype(f(std::declval<T>())), E> {
        if (_is_ok) {
            return {internal::OkWrapper<decltype(f(std::declval<T>()))>{f(_ok)}};
        } else {
            return {internal::ErrorWrapper<E>{_error}};
        }
    }

    /// @brief Transform the error value using a function
    /// @tparam F Callable type (auto‑deduced)
    /// @param f Mapping function: E -> W
    /// @return Result<T, W> containing the mapped error on failure,
    ///         otherwise the original success value.
    template<typename F> [[nodiscard]] auto mapError(F &&f) const noexcept -> Result<T, decltype(f(std::declval<E>()))> {
        if (_is_ok) {
            return {internal::OkWrapper<T>{_ok}};
        } else {
            return {internal::ErrorWrapper<decltype(f(std::declval<E>()))>{f(_error)}};
        }
    }

private:
    union {
        T _ok;
        E _error;
    };
    bool _is_ok;

    friend struct ::kf::internal::ResultErrorController<Result<T, E>, E>;

    [[nodiscard]] bool isErrorImpl() const noexcept { return not _is_ok; }

    [[nodiscard]] const E &getErrorImpl() const noexcept { return _error; }
};

/// @brief Specialisation of Result for void success value
/// @tparam E Type of error value
template<typename E> struct Result<void, E> final : internal::ResultErrorController<Result<void, E>, E> {

    /// @brief Construct from OkWrapper<void>
    /// @see kf::ok()
    constexpr Result(internal::OkWrapper<void>) noexcept :
        _is_error{false} {}

    /// @brief Construct from ErrorWrapper
    /// @see kf::error(E)
    constexpr Result(internal::ErrorWrapper<E> error) noexcept :
        _is_error{true}, _error{std::move(error.value)} {}

    ~Result() noexcept {
        if (_is_error) {
            _error.~E();
        }
    }

    /// @brief Check if result is successful
    [[nodiscard]] constexpr bool isOk() const noexcept { return not _is_error; }

    /// @brief Transform the error value using a function
    /// @tparam F Callable type (auto‑deduced)
    /// @param f Mapping function: E -> W
    /// @return Result<T, W> containing the mapped error on failure,
    ///         otherwise the original success value.
    template<typename F> [[nodiscard]] auto mapError(F &&f) const noexcept -> Result<void, decltype(f(std::declval<E>()))> {
        if (_is_error) {
            return {internal::ErrorWrapper<decltype(f(std::declval<E>()))>{f(_error)}};
        } else {
            return {internal::OkWrapper<void>{}};
        }
    }

private:
    E _error;
    bool _is_error;

    friend struct ::kf::internal::ResultErrorController<Result<void, E>, E>;

    [[nodiscard]] bool isErrorImpl() const noexcept { return _is_error; }

    [[nodiscard]] const E &getErrorImpl() const noexcept { return _error; }
};

/// @brief Create a successful Result (value)
/// @tparam T Type of the value (auto‑deduced)
/// @param value The success value
/// @return internal::OkWrapper<T> for implicit conversion to Result<T, E>
template<typename T> constexpr auto ok(T &&value) noexcept -> internal::OkWrapper<std::decay_t<T>> {
    return {std::forward<T>(value)};
}

/// @brief Create a successful Result<void,  E>
/// @return internal::OkWrapper<void> for implicit conversion to Result<void, E>
constexpr auto ok() noexcept -> internal::OkWrapper<void> {
    return {};
}

/// @brief Create an error Result
/// @tparam E Type of the error (auto‑deduced)
/// @param error The error value
/// @return internal::ErrorWrapper<E> for implicit conversion to Result<T, E>
template<typename E> constexpr auto error(E &&error) noexcept -> internal::ErrorWrapper<std::decay_t<E>> {
    return {std::forward<E>(error)};
}

}// namespace kf
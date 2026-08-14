// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    Result.hpp

#pragma once

#include <cstdlib>
#include <type_traits>
#include <utility>

#include "kf/core.hpp"

namespace kf {

namespace internal {

/// @brief Base class for result error handling logic (CRTP)
template<typename Impl, trivial E> struct ResultErrorController {

    /// @brief Check if result contains an error
    [[nodiscard]] constexpr bool isError() const noexcept {
        return static_cast<Impl const *>(this)->isErrorImpl();
    }

    /// @brief Get the stored error (panic if not error)
    [[nodiscard]] E const &error() const noexcept {
        if (isError()) { return static_cast<Impl const *>(this)->getErrorImpl(); }
        abort();
    }
};

/// @brief Internal wrapper for a success value (used with kf::ok)
template<typename T> struct ResultValueWrapper {
    T value;
};

/// @brief Specialization of ResultValueWrapper for void
template<> struct ResultValueWrapper<void> {};

/// @brief Internal wrapper for an error value (used with kf::error)
template<typename E> struct ResultErrorWrapper {
    E value;
};

}// namespace internal

struct ResultTag {};

/// @brief Result type that can hold either a value or an error
/// @tparam T Type of successful result value
/// @tparam E Type of error value
/// @note Embedded‑friendly alternative to exceptions.
///       Use kf::ok() and kf::error() to create instances.
/// @see kf::ok, kf::error
template<typename T, typename E> struct Result final :

    ResultTag,
    internal::ResultErrorController<Result<T, E>, E>

{
    using ValueType = T;

    /// @brief Construct from ResultValueWrapper
    /// @see kf::ok(T)
    constexpr Result(internal::ResultValueWrapper<T> ok) noexcept :
        _is_ok{true}, _ok{std::move(ok.value)} {}

    /// @brief Construct from ResultErrorWrapper
    /// @see kf::error(E)
    constexpr Result(internal::ResultErrorWrapper<E> error) noexcept :
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

    /// @brief Get the stored value (abort if is error)
    [[nodiscard]] T &ok() & noexcept {
        if (this->isError()) { abort(); }
        return _ok;
    }

    [[nodiscard]] T ok() && noexcept {
        if (this->isError()) { abort(); }
        return std::move(_ok);
    }

    /// @brief Get the stored value (const overload)
    [[nodiscard]] T const &ok() const & noexcept {
        return const_cast<Result *>(this)->ok();
    }

    /// @brief Transform the success value using a function
    /// @param f Mapping function: T -> U
    /// @return Result<U, E> containing the mapped value on success,
    ///         otherwise the original error.
    [[nodiscard]] auto map(auto &&f) const noexcept -> Result<decltype(f(std::declval<T>())), E> {
        if (_is_ok) {
            if constexpr (std::is_void_v<decltype(f(std::declval<T>()))>) {
                f(_ok);
                return {internal::ResultValueWrapper<void>{}};
            } else {
                return {internal::ResultValueWrapper<decltype(f(std::declval<T>()))>{f(_ok)}};
            }
        } else {
            return {internal::ResultErrorWrapper<E>{_error}};
        }
    }

    /// @brief Transform the error value using a function
    /// @param f Mapping function: E -> W
    /// @return Result<T, W> containing the mapped error on failure,
    ///         otherwise the original success value.
    [[nodiscard]] auto mapError(auto &&f) const noexcept -> Result<T, decltype(f(std::declval<E>()))> {
        if (_is_ok) {
            return {internal::ResultValueWrapper<T>{_ok}};
        } else {
            return {internal::ResultErrorWrapper<decltype(f(std::declval<E>()))>{f(_error)}};
        }
    }

private:
    union {
        T _ok;
        E _error;
    };
    bool _is_ok;

    friend struct ::kf::internal::ResultErrorController<Result<T, E>, E>;

    [[nodiscard]] bool isErrorImpl() const noexcept {
        return not _is_ok;
    }

    [[nodiscard]] E const &getErrorImpl() const noexcept {
        return _error;
    }
};

/// @brief Specialization of Result for void success value
/// @tparam E Type of error value
template<typename E> struct Result<void, E> final :

    ResultTag,
    internal::ResultErrorController<Result<void, E>, E>

{
    using ValueType = void;

    /// @brief Construct from ResultValueWrapper<void>
    /// @see kf::ok()
    constexpr Result(internal::ResultValueWrapper<void>) noexcept :
        _is_error{false} {}

    /// @brief Construct from ResultErrorWrapper
    /// @see kf::error(E)
    constexpr Result(internal::ResultErrorWrapper<E> error) noexcept :
        _is_error{true}, _error{std::move(error.value)} {}

    ~Result() noexcept {
        if (_is_error) {
            _error.~E();
        }
    }

    constexpr void ok() const noexcept {}

    /// @brief Check if result is successful
    [[nodiscard]] constexpr bool isOk() const noexcept {
        return not _is_error;
    }

    /// @brief Transform the error value using a function
    /// @param f Mapping function: E -> W
    /// @return Result<T, W> containing the mapped error on failure,
    ///         otherwise the original success value.
    [[nodiscard]] auto mapError(auto &&f) const noexcept -> Result<void, decltype(f(std::declval<E>()))> {
        if (_is_error) {
            return {internal::ResultErrorWrapper<decltype(f(std::declval<E>()))>{f(_error)}};
        } else {
            return {internal::ResultValueWrapper<void>{}};
        }
    }

private:
    E _error;
    bool _is_error;

    friend struct ::kf::internal::ResultErrorController<Result<void, E>, E>;

    [[nodiscard]] bool isErrorImpl() const noexcept {
        return _is_error;
    }

    [[nodiscard]] E const &getErrorImpl() const noexcept {
        return _error;
    }
};

/// @brief Create a successful Result (value)
/// @param value The success value
/// @return internal::ResultValueWrapper<T> for implicit conversion to Result<T, E>
constexpr auto ok(auto &&value) noexcept -> internal::ResultValueWrapper<std::decay_t<decltype(value)>> {
    return {std::forward<decltype(value)>(value)};
}

/// @brief Create a successful Result<void,  E>
/// @return internal::ResultValueWrapper<void> for implicit conversion to Result<void, E>
constexpr auto ok() noexcept -> internal::ResultValueWrapper<void> {
    return {};
}

/// @brief Create an error Result
/// @param error The error value
/// @return internal::ResultErrorWrapper<E> for implicit conversion to Result<T, E>
constexpr auto error(auto &&error) noexcept -> internal::ResultErrorWrapper<std::decay_t<decltype(error)>> {
    return {std::forward<decltype(error)>(error)};
}

}// namespace kf

/// @brief Early-return macro for Result error propagation
/// @param __expression__ Expression of type `Result<T, E>`
/// @return The success value `T`, or returns the error from the enclosing function
/// @note Expression evaluated once. Requires GNU statement expressions.
#define KF_TRY(__expression__)                                                                                              \
    ({                                                                                                                      \
        auto result = (__expression__);                                                                                     \
        static_assert(std::is_base_of_v<::kf::ResultTag, std::decay_t<decltype(result)>>, "KF_TRY requires a Result type"); \
        if (result.isError()) {                                                                                             \
            return ::kf::error(result.error());                                                                             \
        }                                                                                                                   \
        result.ok();                                                                                                        \
    })

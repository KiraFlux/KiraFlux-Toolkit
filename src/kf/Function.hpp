// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once
#include "kf/core/config.hpp"

#if kf_port_has_functional
#include <functional>

namespace kf {

/// @brief Function wrapper for platforms with standard library support
/// @tparam F Function signature (e.g., void(int, float))
/// @note Alias to std::function when available
template<typename F> using Function = std::function<F>;
}// namespace kf

#else

#include "stdlib.h"// NOLINT(*-deprecated-headers)

#include "kf/core/utility.hpp"

namespace kf {

/// @brief Forward declaration for function wrapper
/// @tparam T Function signature type
template<typename T> class Function;

/// @brief Lightweight function wrapper for embedded platforms without std::function
/// @tparam R Return type
/// @tparam Args Argument types
/// @note Simplified implementation storing only function pointer (no type erasure)
template<typename R, typename... Args>
class Function<R(Args...)> {
private:
    using func_ptr_t = R (*)(Args...);///< Function pointer type
    func_ptr_t func_ptr = nullptr;    ///< Stored function pointer

public:
    /// @brief Default constructor (empty function)
    Function() noexcept = default;

    /// @brief Construct from nullptr (empty function)
    /// @note Allows initialization with nullptr
    Function(nullptr_t) noexcept {}// NOLINT(*-explicit-constructor)

    /// @brief Construct from function pointer
    /// @param f Function pointer to store
    Function(func_ptr_t f) noexcept :// NOLINT(*-explicit-constructor)
        func_ptr(f) {}

    /// @brief Construct from callable object (function pointer only)
    /// @tparam F Callable type (must be convertible to function pointer)
    /// @param f Callable object to store
    /// @note Only supports function pointers, not arbitrary callable objects
    template<typename F> Function(F f) noexcept :// NOLINT(*-explicit-constructor)
        func_ptr(static_cast<func_ptr_t>(f)) {}

    /// @brief Copy constructor
    /// @param other Function to copy
    Function(const Function &other) noexcept = default;

    /// @brief Move constructor
    /// @param other Function to move from
    Function(Function &&other) noexcept = default;

    /// @brief Assign nullptr (reset to empty)
    /// @return Reference to this function
    Function &operator=(nullptr_t) noexcept {
        func_ptr = nullptr;
        return *this;
    }

    /// @brief Assign function pointer
    /// @param f Function pointer to assign
    /// @return Reference to this function
    Function &operator=(func_ptr_t f) noexcept {
        func_ptr = f;
        return *this;
    }

    /// @brief Copy assignment
    /// @param other Function to copy
    /// @return Reference to this function
    Function &operator=(const Function &other) noexcept = default;

    /// @brief Move assignment
    /// @param other Function to move from
    /// @return Reference to this function
    Function &operator=(Function &&other) noexcept = default;

    /// @brief Assign callable object (function pointer only)
    /// @tparam F Callable type (must be convertible to function pointer)
    /// @param f Callable object to assign
    /// @return Reference to this function
    template<typename F> Function &operator=(F f) noexcept {
        func_ptr = static_cast<func_ptr_t>(f);
        return *this;
    }

    /// @brief Call stored function
    /// @param args Arguments to pass to function
    /// @return Result of function call
    /// @note Returns default-constructed R if function is empty
    R operator()(Args... args) const {
        if (func_ptr) {
            return func_ptr(args...);
        } else {
            return R();
        }
    }

    /// @brief Check if function contains a callable
    /// @return true if function is non-empty
    explicit operator bool() const noexcept {
        return func_ptr != nullptr;
    }

    /// @brief Compare with nullptr (empty function)
    /// @return true if function is empty
    bool operator==(nullptr_t) const noexcept {
        return func_ptr == nullptr;
    }

    /// @brief Compare with nullptr (non-empty function)
    /// @return true if function is non-empty
    bool operator!=(nullptr_t) const noexcept {
        return func_ptr != nullptr;
    }

    /// @brief Compare with function pointer
    /// @param f Function pointer to compare with
    /// @return true if stored pointer equals f
    bool operator==(func_ptr_t f) const noexcept {
        return func_ptr == f;
    }

    /// @brief Compare with function pointer
    /// @param f Function pointer to compare with
    /// @return true if stored pointer differs from f
    bool operator!=(func_ptr_t f) const noexcept {
        return func_ptr != f;
    }
};

}// namespace kf

#endif
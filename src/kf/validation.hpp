// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Logger.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf {

/// @brief Validation helper for checking conditions and collecting errors
/// @note Used to validate configuration and object states
struct Validator final : mixin::NonCopyable {

    /// @brief Check a validation condition and log result.
    /// @param logger          The logger instance to use for output.
    /// @param condition_ok    True if condition passes, false otherwise.
    /// @param condition_string String representation of the condition (used in logs).
    void check(const Logger &logger, bool condition_ok, memory::StringView condition_string) noexcept {
        if (condition_ok) {
            logger.info(condition_string);
        } else {
            logger.error(condition_string);
            _errors_total += 1;
        }
    }

    /// @brief Check if all validation conditions passed
    /// @return true if no errors were recorded, false otherwise
    [[nodiscard]] bool passed() const noexcept {
        return _errors_total == 0;
    }

private:
    usize _errors_total{0};///< Count of failed validation checks
};

/// @brief CRTP base class for objects that can validate themselves
/// @tparam Impl Concrete type implementing validation
/// @note Derived classes must implement `void checkImpl(Validator &) const noexcept` method
template<typename Impl> struct Validatable {

    /// @brief Perform validation
    void check(Validator &validator) const noexcept {
        static_cast<const Impl *>(this)->checkImpl(validator);
    }
};

}// namespace kf

#define KF_IMPL_VALIDATABLE(...) friend struct __VA_ARGS__

/// @brief Macro to simplify validation check calls
#define KF_VALIDATOR_CHECK(validator_instance, logger_instance, condition) validator_instance.check(logger_instance, condition, #condition)
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Logger.hpp"

namespace kf {

/// @brief Validation helper for checking conditions and collecting errors
/// @note Used to validate configuration and object states
struct Validator {

private:
    usize errors{0};///< Count of failed validation checks

public:
    /// @brief Check a validation condition and log result
    void check(const Logger &logger, bool condition_ok, StringView condition_string) noexcept {
        if (condition_ok) {
            logger.info(condition_string);
        } else {
            logger.error(condition_string);
            errors += 1;
        }
    }

    /// @brief Check if all validation conditions passed
    /// @return true if no errors were recorded, false otherwise
    [[nodiscard]] bool passed() const noexcept {
        return errors == 0;
    }
};

/// @brief CRTP base class for objects that can validate themselves
/// @tparam Impl Concrete type implementing validation
/// @note Derived classes must implement check(Validator&) method
template<typename Impl> struct Validatable {
    /// @brief Perform validation and return result
    /// @return true if object passed all validation checks
    [[nodiscard]] bool check() const noexcept {
        Validator validator{};
        static_cast<const Impl *>(this)->checkImpl(validator);
        return validator.passed();
    }
};

}// namespace kf

/// @brief Macro to simplify validation check calls
#define kf_Validator_check(validator_instance, logger_instance, condition) validator_instance.check(logger_instance, condition, #condition)
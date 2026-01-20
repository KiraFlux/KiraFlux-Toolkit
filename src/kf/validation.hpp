// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Logger.hpp"
#include "kf/core/attributes.hpp"

namespace kf {

/// @brief Validation helper for checking conditions and collecting errors
/// @note Used to validate configuration settings and object states
struct Validator {

private:
    int errors{0};///< Count of failed validation checks

public:
    /// @brief Check a validation condition and log result
    /// @param scope Source file or context for the check
    /// @param condition_ok Boolean result of the condition
    /// @param condition_string String representation of the condition (for logging)
    void check(const char *scope, bool condition_ok, const char *condition_string) {
        const auto &logger = kf::Logger::instance();

        if (condition_ok) {
            logger.log(
                "[  OK  ]",
                scope,
                ": %s", condition_string);
        } else {
            logger.log(
                "[ FAIL ]",
                scope,
                ": %s", condition_string);

            errors += 1;
        }
    }

    /// @brief Check if all validation conditions passed
    /// @return true if no errors were recorded, false otherwise
    kf_nodiscard bool passed() const {
        return errors == 0;
    }
};

/// @brief CRTP base class for objects that can validate themselves
/// @tparam Impl Concrete type implementing validation
/// @note Derived classes must implement check(Validator&) method
template<typename Impl> struct Validable {
    /// @brief Perform validation and return result
    /// @return true if object passed all validation checks
    kf_nodiscard bool isValid() const {
        Validator validator{};

        static_cast<const Impl *>(this)->check(validator);

        return validator.passed();
    }
};

}// namespace kf

/// @brief Macro to simplify validation check calls
/// @param validator_instance Validator object to report to
/// @param condition Boolean expression to validate
/// @note Automatically captures file context and condition string
#define kf_Validator_check(validator_instance, condition) validator_instance.check(__FILE__, condition, #condition)
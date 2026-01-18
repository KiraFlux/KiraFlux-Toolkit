#pragma once

#include "kf/Logger.hpp"
#include "kf/attributes.hpp"

namespace kf {
namespace tools {

struct Validator {

private:
    int errors{0};

public:
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

    kf_nodiscard bool passed() const {
        return errors == 0;
    }
};

template<typename Impl> struct Validable {
    kf_nodiscard bool isValid() const {
        Validator validator{};

        static_cast<const Impl *>(this)->check(validator);

        return validator.passed();
    }
};

}// namespace tools
}// namespace kf

#define kf_Validator_check(validator_instance, condition) validator_instance.check(__FILE__, condition, #condition)

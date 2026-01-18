#pragma once

// deprecated header for AVR
#include <stdlib.h>// NOLINT(*-deprecated-headers)

#include "kf/attributes.hpp"

namespace kf {

template<typename T> struct Option {

private:
    union {
        T val;
        char dummy;
    };
    bool engaged;

public:
    constexpr Option(const T &value) :// NOLINT(*-explicit-constructor)
        engaged{true}, val{value} {}

    constexpr Option() :
        engaged{false}, dummy{0} {}

    /// @brief Option contains value
    kf_nodiscard bool hasValue() const { return engaged; }

    /// @brief Try to get optional value
    kf_nodiscard T &value() {
        if (engaged) {
            return val;
        } else {
            abort();
        }
    }

    /// @brief Safe alternative that doesn't terminate on fail
    kf_nodiscard T valueOr(const T &default_value) const {
        return engaged ? val : default_value;
    }
};

}// namespace kf
#pragma once

#include "kf/Option.hpp"
#include "kf/attributes.hpp"

namespace kf {

/// @brief Result or Error
template<typename T, typename E> struct Result {

private:
    bool is_ok;

    union {
        T value;
        E err;
    };

public:
    /// @brief Create Successful result
    constexpr Result(T val) :// NOLINT(*-explicit-constructor)
        is_ok{true}, value{val} {}

    /// @brief Create Error result
    constexpr Result(E error) :// NOLINT(*-explicit-constructor)
        is_ok{false}, err{error} {}

    /// @brief Is contains value
    kf_nodiscard bool isOk() const { return is_ok; }

    /// @brief Is contains error
    kf_nodiscard bool isError() const { return not is_ok; }

    /// @brief Optional Successful Value
    Option<T> ok() const {
        if (is_ok) {
            return {value};
        } else {
            return {};
        }
    }

    /// @brief Optional Error Value
    Option<E> error() const {
        if (is_ok) {
            return {};
        } else {
            return {err};
        }
    }
};

// void-value specification
template<typename E> struct Result<void, E> {

private:
    bool is_ok;
    E err;

public:
    /// @brief Create Successful result
    constexpr Result() :
        is_ok{true} {}

    /// @brief Create Error result
    constexpr Result(E error) :// NOLINT(*-explicit-constructor)
        is_ok{false}, err{error} {}

    /// @brief Is contains value
    kf_nodiscard bool isOk() const { return is_ok; }

    /// @brief Is contains error
    kf_nodiscard bool isError() const { return not is_ok; }

    /// @brief Optional Error Value
    Option<E> error() const {
        if (is_ok) {
            return {};
        } else {
            return {err};
        }
    }
};

}// namespace kf
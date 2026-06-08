// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Function.hpp"
#include "kf/NoneType.hpp"
#include "kf/Option.hpp"

namespace kf::internal {

template<typename Signature> struct CallbackedController {
    using CallbackType = Function<Signature>;

    /// @brief Set callback from optional function
    void callback(Option<CallbackType> optional_function) noexcept {
        _callback_function = std::move(optional_function);
    }

    /// @brief Set callback from function object
    void callback(CallbackType function) noexcept {
        _callback_function = some(std::move(function));
    }

    /// @brief Set callback as None
    void callback(NoneType) noexcept {
        _callback_function.reset();
    }

protected:
    Option<CallbackType> _callback_function{none};
};

}// namespace kf::internal

namespace kf::mixin {

/// @brief Tag specifies that target struct support Callbacked mixin
struct CallbackedTag {};

/// @brief Adds Callback
/// @tparam T Callback argument type
template<typename T> struct Callbacked : CallbackedTag, internal::CallbackedController<void(T)> {

    /// @brief Invoke callback function if is some
    /// @param value callback function argument
    void invoke(T value) const noexcept {
        if (this->_callback_function.isSome()) {
            this->_callback_function.unwrap()(value);
        }
    }
};

/// @brief Adds Callback with function without arguments
template<> struct Callbacked<void> : CallbackedTag, internal::CallbackedController<void()> {

    /// @brief Invoke callback function if is some
    void invoke() const noexcept {
        if (this->_callback_function.isSome()) {
            this->_callback_function.unwrap()();
        }
    }
};

}// namespace kf::mixin
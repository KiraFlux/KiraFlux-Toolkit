// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Function.hpp"
#include "kf/NoneType.hpp"
#include "kf/Option.hpp"

namespace kf::mixin {

/// @brief Tag specifies that target struct support Callbacked mixin
struct CallbackedTag {};

/// @brief Adds Callback
/// @tparam Args Callback arguments
template<typename... Args> struct Callbacked : CallbackedTag {
    using CallbackType = Function<void(Args...)>;

    /// @brief Set callback from optional function
    void callback(Option<CallbackType> optional_function) noexcept {
        _callback_function = std::move(optional_function);
    }

    /// @brief Set callback from function object
    template<typename F> void callback(F &&function) noexcept {
        _callback_function = some(CallbackType{std::forward<F>(function)});
    }

    /// @brief Set callback as None
    void callback(NoneType) noexcept {
        _callback_function.reset();
    }

protected:
    /// @brief Invoke callback function if is some
    /// @param value callback function argument
    void invoke(Args... args) const noexcept {
        if (this->_callback_function.isSome()) {
            this->_callback_function.unwrap()(args...);
        }
    }

private:
    Option<CallbackType> _callback_function{none};
};

}// namespace kf::mixin
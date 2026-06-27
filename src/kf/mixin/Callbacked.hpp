// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <type_traits>
#include <utility>

#include "kf/Function.hpp"
#include "kf/NoneType.hpp"
#include "kf/Option.hpp"

namespace kf::mixin {

/// @brief Tag specifies that target struct support Callbacked mixin
struct CallbackedTag {};

/// @brief Adds Callback
/// @tparam Signature Callback signature
template<typename Signature> struct Callbacked : CallbackedTag {

    /// @brief Callback Function type
    using FunctionType = Function<Signature>;

    /// @brief Set callback from optional function
    void callback(Option<FunctionType> optional_function) noexcept {
        _callback_function = std::move(optional_function);
    }

    /// @brief Set callback from function object
    template<typename F> void callback(F &&function) noexcept {
        _callback_function = some(FunctionType{std::forward<F>(function)});
    }

    /// @brief Set callback as None
    void callback(NoneType) noexcept {
        _callback_function.reset();
    }

protected:
    /// @brief Invoke callback function if is some
    /// @param value callback function argument
    template<typename... CallArgs> auto invoke(CallArgs &&...args) const noexcept -> typename FunctionType::ReturnType {
        if (this->_callback_function.isSome()) {
            if constexpr (std::is_void_v<typename FunctionType::ReturnType>) {
                this->_callback_function.unwrap()(std::forward<CallArgs>(args)...);
            } else {
                return this->_callback_function.unwrap()(std::forward<CallArgs>(args)...);
            }
        }
    }

private:
    Option<FunctionType> _callback_function{none};
};

}// namespace kf::mixin
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

/// @file    mixin/Callbacked.hpp
/// @brief   Adds an optional callback (Function) with invoke().

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
    void callback(auto &&function) noexcept {
        _callback_function = some(FunctionType{std::forward<decltype(function)>(function)});
    }

    /// @brief Set callback as None
    void callback(NoneType) noexcept {
        _callback_function.reset();
    }

    /// @brief Get callback (lvalue)
    decltype(auto) callback() & noexcept {
        return _callback_function;
    }

    /// @brief Get callback (lvalue const )
    decltype(auto) callback() const & noexcept {
        return _callback_function;
    }

    /// @brief Get callback (rvalue)
    decltype(auto) callback() && noexcept {
        return std::move(_callback_function);
    }

protected:
    /// @brief Invoke callback function if is some
    /// @param value callback function argument
    auto invoke(auto &&...args) const noexcept -> Option<typename FunctionType::ReturnType> {
        if (this->_callback_function.isSome()) {
            if constexpr (std::is_void_v<typename FunctionType::ReturnType>) {
                this->_callback_function.unwrap()(std::forward<decltype(args)>(args)...);
                return some();
            } else {
                return some(this->_callback_function.unwrap()(std::forward<decltype(args)>(args)...));
            }
        } else {
            return none;
        }
    }

private:
    Option<FunctionType> _callback_function{none};
};

}// namespace kf::mixin
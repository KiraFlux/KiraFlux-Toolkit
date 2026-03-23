// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Function.hpp"

namespace kf::mixin {

struct CallbackedTag {};

/// @brief Adds Callback
/// @tparam T
template<typename T> struct Callbacked : CallbackedTag {
    using CallbackType = Function<void(T)>;

    void callback(CallbackType &&function) noexcept {
        _callback_function = std::move(function);
    }

protected:
    void invoke(T value) noexcept {
        if (_callback_function) {
            _callback_function(value);
        }
    }

private:
    CallbackType _callback_function{nullptr};
};

template<> struct Callbacked<void> : CallbackedTag {
    using CallbackType = Function<void()>;

    void callback(CallbackType &&function) noexcept {
        _callback_function = std::move(function);
    }

protected:
    void invoke() noexcept {
        if (_callback_function) {
            _callback_function();
        }
    }

private:
    CallbackType _callback_function{nullptr};
};

}// namespace kf::mixin
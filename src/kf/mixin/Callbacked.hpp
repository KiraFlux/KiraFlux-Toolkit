// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/Function.hpp"

namespace kf::mixin {

struct CallbackedTag {};

/// @brief Adds Callback
/// @tparam T
template<typename T> struct Callbacked : CallbackedTag {
    using HandlerType = Function<void(T)>;

    void handler(HandlerType &&new_handler) noexcept {
        _handler = std::move(new_handler);
    }

protected:
    void invoke(T value) noexcept {
        if (_handler) {
            _handler(value);
        }
    }

private:
    HandlerType _handler{nullptr};
};

template<> struct Callbacked<void> : CallbackedTag {
    using HandlerType = Function<void()>;

    void handler(HandlerType &&new_handler) noexcept {
        _handler = std::move(new_handler);
    }

protected:
    void invoke() noexcept {
        if (_handler) {
            _handler();
        }
    }

private:
    HandlerType _handler{nullptr};
};

}// namespace kf::mixin
// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Callbacked.hpp"

namespace kf::mixin {

struct ValueCallbackedTag {};

/// @brief Adds value: T
template<typename T> struct ValueCallbacked : ValueCallbackedTag, Callbacked<T> {
    constexpr explicit ValueCallbacked(T value) noexcept : _value{value} {}

    [[nodiscard]] constexpr T value() const noexcept { return _value; }

    void value(T new_value) noexcept {
        if (_value != new_value) {
            _value = new_value;
            Callbacked<T>::invoke(_value);
        }
    }

private:
    T _value;
};

}// namespace kf::mixin
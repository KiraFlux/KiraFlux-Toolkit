// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/mixin/Callbacked.hpp"

namespace kf::mixin {

struct ValueCallbackedTag {};

/// @brief Adds value: T
/// @tparam T The type of the stored value.
template<typename T> struct ValueCallbacked : ValueCallbackedTag, Callbacked<void(T)> {

    /// @brief Constructs with an initial value.
    /// @param value The initial value.
    explicit constexpr ValueCallbacked(T value) noexcept :
        _value{value} {}

    /// @brief Returns the current value.
    /// @return The stored value
    [[nodiscard]] constexpr T value() const noexcept {
        return _value;
    }

    /// @brief Sets a new value and invokes the callback if it changed.
    /// @param new_value The new value to set.
    void value(T new_value) noexcept {
        if (_value != new_value) {
            _value = new_value;
            this->invoke(_value);
        }
    }

private:
    T _value;
};

}// namespace kf::mixin
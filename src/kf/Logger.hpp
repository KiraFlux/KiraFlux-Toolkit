// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/StringView.hpp"
#include "kf/mixin/NonCopyable.hpp"

namespace kf {

/// @brief Logging system for embedded applications
struct Logger : mixin::NonCopyable {

    using WriteHandler = void (*)(StringView);

    /// @brief Current output handler (nullptr disables logging)
    inline static WriteHandler writer{nullptr};

    explicit constexpr Logger(StringView key) noexcept :
        _key{key} {}

#define MAKE(__level__)                                  \
    void __level__(StringView message) const noexcept {  \
        write(StringView{":" #__level__ "] "}, message); \
    }

    MAKE(info)

    MAKE(warn)

    MAKE(error)

    MAKE(debug)

#undef MAKE

private:
    StringView _key;

    void write(StringView level, StringView message) const noexcept {
        if (nullptr != writer) {
            writer("[");
            writer(_key);
            writer(level);
            writer(message);
            writer("\n");
        }
    }
};

}// namespace kf
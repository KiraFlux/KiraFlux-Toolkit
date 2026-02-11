// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include "kf/attributes.hpp"
#include "kf/memory/ArrayString.hpp"
#include "kf/memory/StringView.hpp"

namespace kf {

/// @brief Logging system for embedded applications
struct Logger final {
    using WriteHandler = void (*)(StringView);

    static WriteHandler writer;///< Current output handler (nullptr disables logging)

private:
    const StringView key;

    constexpr explicit Logger(StringView key) noexcept :
        key{key} {}

public:
    template<usize N> kf_nodiscard static constexpr Logger create(const char (&key)[N]) noexcept {
        return Logger{StringView{key, N - 1}};
    }

#define MAKE(__entry_name__)                                       \
    void __entry_name__(const StringView message) const noexcept { \
        write(StringView{#__entry_name__}, message);               \
    }

    MAKE(info)

    MAKE(warn)

    MAKE(error)

    MAKE(debug)

#undef MAKE

private:
    void write(const StringView level, const StringView message) const noexcept {
        if (writer == nullptr) { return; }

        ArrayString<32> buffer{};

        const auto now = millis();
        (void) buffer.append(static_cast<i32>(now));
        (void) buffer.append(" [");
        (void) buffer.append(key);
        (void) buffer.push(':');
        (void) buffer.append(level);
        (void) buffer.append("] ");
        writer(buffer.view());
        writer(message);
        writer("\n");
    }
};

Logger::WriteHandler Logger::writer{nullptr};

}// namespace kf

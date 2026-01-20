// Copyright (c) 2026 KiraFlux
// SPDX-License-Identifier: MIT

#pragma once

#include <stdarg.h>

#include "kf/core/aliases.hpp"
#include "kf/memory/Array.hpp"

namespace kf {

/// @brief Fixed-size character array used as string storage
/// @tparam N Maximum string length including null terminator
/// @note Uses fixed-size array to avoid dynamic allocation in embedded systems
template<usize N> using ArrayString = kf::Array<char, N>;

/// @brief Format string into fixed-size character array using vsnprintf
/// @tparam N Size of destination array (must be > 0 for effective formatting)
/// @param destination ArrayString to write formatted result into
/// @param format printf-style format string
/// @param ... Variable arguments matching format specifiers
/// @note Guarantees null termination and prevents buffer overflow
template<usize N> void formatTo(ArrayString<N> &destination, const char *format, ...) {
    if (N == 0) { return; }

    va_list args;
    va_start(args, format);

    vsnprintf(destination.data(), N, format, args);
    destination[N - 1] = '\0';

    va_end(args);
}

}// namespace kf
#pragma once

#include <stdarg.h>

#include "kf/aliases.hpp"
#include "kf/array.hpp"

namespace kf {

struct String {};

template<usize N> using ArrayString = kf::array<char, N>;

/// @brief Форматировать Массив-строку через <code>vsnprintf</code>
template<usize N> void formatTo(ArrayString<N> &destination, const char *format, ...) {
    if (N == 0) { return; }

    va_list args;
    va_start(args, format);

    vsnprintf(destination.data(), N, format, args);
    destination[N - 1] = '\0';

    va_end(args);
}

}// namespace kf

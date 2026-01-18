#pragma once

#include "kf/aliases.hpp"

namespace kf {
namespace tools {

/// Экспоненциальный фильтр
template<typename T> struct ExponentialFilter {
    f32 k;
    T filtered;

    constexpr explicit ExponentialFilter(f32 k, T init_value = T{}) :
        k{k}, filtered{init_value} {}

    const T &calc(const T &value) {
        filtered += (value - filtered) * k;
        return filtered;
    }
};

}// namespace tools
}// namespace kf

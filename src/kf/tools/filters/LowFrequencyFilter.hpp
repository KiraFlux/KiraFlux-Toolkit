#pragma once

#include "kf/aliases.hpp"

namespace kf {
namespace tools {

/// @brief Низкочастотный фильтры
/// @tparam T Скаляр
template<typename T> struct LowFrequencyFilter {

private:
    const f32 alpha;
    const f32 one_minus_alpha{1.0f - alpha};
    T filtered{};
    bool first_step{false};

public:
    explicit LowFrequencyFilter(f32 alpha) noexcept :
        alpha{alpha} {}

    const T &calc(const T &x) noexcept {
        if (first_step) {
            first_step = false;
            filtered = x;
            return filtered;
        }

        if (alpha == 1.0) {
            filtered = x;
            return filtered;
        }

        filtered = filtered * one_minus_alpha + x * alpha;
        return filtered;
    }

    /// @brief Сбросить значение фильтра
    void reset() {
        first_step = true;
    }
};

}// namespace tools
}// namespace kf

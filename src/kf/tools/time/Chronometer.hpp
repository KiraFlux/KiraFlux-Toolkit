#pragma once

#include <Arduino.h>

#include "kf/units.hpp"

namespace kf {
namespace tools {

/// @brief Хронометр
struct Chronometer final {

private:
    /// @brief Момент предыдущего измерения
    Microseconds last{micros()};

public:
    /// @brief Рассчитать дельту между вызовами
    /// @param now Время в данный момент
    Seconds calc(Microseconds now) noexcept {
        const auto delta = now - last;
        last = now;
        return static_cast<Seconds>(delta) * 1e-6f;
    }
};

}// namespace tools
}// namespace kf

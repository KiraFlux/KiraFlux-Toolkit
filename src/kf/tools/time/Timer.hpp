#pragma once

#include "kf/units.hpp"

namespace kf {
namespace tools {

/// @brief Таймер
struct Timer {

    /// @brief Период срабатывания таймера
    Milliseconds period;

private:
    /// @brief Момент предыдущего срабатывания
    Milliseconds last{0};

public:
    explicit Timer(Milliseconds period) :
        period{period} {}

    explicit Timer(Hertz frequency) :
        period{static_cast<Milliseconds>(1000 / frequency)} {}

    /// @brief Прошел ли период
    /// @param now Текущее время
    bool ready(Milliseconds now) {
        const auto delta = now - last;

        if (delta < period) {
            return false;
        }

        last = now;

        return true;
    }
};

}// namespace tools
}// namespace kf
